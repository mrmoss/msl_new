//Windows Dependencies:
//		-lhid -lpthread

//Linux Dependencies:
//		-lpthread

//OSX Dependencies:
//		-framework IOKit -framework CoreFoundation

#include "joystick.hpp"

#include <chrono>
#include <stdexcept>
#include <thread>

#if(defined(_WIN32)&&!defined(__CYGWIN__))

#include <Hidsdi.h>

#define HID_USAGE_NONE_DESKTOP			0xff
#define HID_USAGE_PAGE_DESKTOP_CONTROLS	0x01
#define HID_USAGE_ALL					0xff
#define HID_USAGE_UNDEFINED				0x00
#define HID_USAGE_POINTER				0x01
#define HID_USAGE_MOUSE					0x02
#define HID_USAGE_RESERVED				0x03
#define HID_USAGE_JOYSTICK				0x04
#define HID_USAGE_GAMEPAD				0x05
#define HID_USAGE_KEYBOARD				0x06
#define HID_USAGE_KEYPAD				0x07
#define HID_USAGE_CONTROLLER			0x08
#define HID_USAGE_TABLET				0x09
#define HID_USAGE_HAT					0x39

static int hid_get_usage(const RAWINPUTDEVICELIST device)
{
	int return_usage=HID_USAGE_NONE_DESKTOP;
	RID_DEVICE_INFO info;
	unsigned int size=sizeof(RID_DEVICE_INFO);
	info.cbSize=size;

	if(GetRawInputDeviceInfo(device.hDevice,RIDI_DEVICEINFO,&info,&size)==sizeof(RID_DEVICE_INFO))
		if(info.hid.usUsagePage==HID_USAGE_PAGE_DESKTOP_CONTROLS)
			return_usage=info.hid.usUsage;

	return return_usage;
}

static std::vector<RAWINPUTDEVICELIST> hid_list(const int usage)
{
	std::vector<RAWINPUTDEVICELIST> devices;

	unsigned int size=0;
	RAWINPUTDEVICELIST* list=nullptr;

	if(GetRawInputDeviceList(nullptr,&size,sizeof(RAWINPUTDEVICELIST))==0)
	{
		list=new RAWINPUTDEVICELIST[size];

		if(GetRawInputDeviceList(list,&size,sizeof(RAWINPUTDEVICELIST))!=size)
			size=0;
	}

	for(unsigned int ii=0;ii<size;++ii)
		if(usage==HID_USAGE_ALL||hid_get_usage(list[ii])==usage)
			devices.push_back(list[ii]);

	delete[](list);

	return devices;
}

static std::string hid_get_name(const RAWINPUTDEVICELIST device)
{
	std::string return_name="";
	unsigned int size=0;

	if(GetRawInputDeviceInfo(device.hDevice,RIDI_DEVICENAME,nullptr,&size)==0)
	{
		char* name_cstr=new char[size];
		size=GetRawInputDeviceInfo(device.hDevice,RIDI_DEVICENAME,name_cstr,&size);
		return_name=std::string(name_cstr,size);

		if(return_name.find("\\??\\")==0)
			return_name[1]='\\';

		delete[](name_cstr);
	}

	return return_name;
}

typedef std::string preparsed_data_t;

static preparsed_data_t get_preparsed_data(const HANDLE& handle)
{
	preparsed_data_t data;
	unsigned int size=0;

	if(GetRawInputDeviceInfo(handle,RIDI_PREPARSEDDATA,nullptr,&size)==0)
	{
		data.resize(size);

		if(GetRawInputDeviceInfo(handle,RIDI_PREPARSEDDATA,(void*)data.c_str(),&size)!=size)
			data.resize(0);
	}

	return data;
}

static HIDP_CAPS get_caps(const HANDLE& handle)
{
	preparsed_data_t preparsed_data=get_preparsed_data(handle);
	HIDP_CAPS caps;
	HidP_GetCaps((PHIDP_PREPARSED_DATA)preparsed_data.data(),&caps);
	return caps;
}

static std::vector<HIDP_VALUE_CAPS> get_axis_caps(const HANDLE& handle)
{
	std::vector<HIDP_VALUE_CAPS> caps;

	try
	{
		preparsed_data_t preparsed_data=get_preparsed_data(handle);
		unsigned long size=get_caps(handle).NumberInputValueCaps;
		HIDP_VALUE_CAPS* ptr=new HIDP_VALUE_CAPS[size];

		if(HidP_GetValueCaps(HidP_Input,ptr,&size,(PHIDP_PREPARSED_DATA)preparsed_data.data())
			!=HIDP_STATUS_SUCCESS)
			return {};

		for(unsigned int ii=0;ii<size;++ii)
		{
			if(ptr[ii].NotRange.Usage==HID_USAGE_HAT)
			{
				ptr[ii].LogicalMin=0;
				ptr[ii].LogicalMax=255;
			}

			caps.push_back(ptr[ii]);
		}

		delete[](ptr);
	}
	catch(...)
	{}

	return caps;
}

static std::vector<HIDP_BUTTON_CAPS> get_button_caps(const HANDLE& handle)
{
	std::vector<HIDP_BUTTON_CAPS> caps;

	try
	{
		preparsed_data_t preparsed_data=get_preparsed_data(handle);
		unsigned long size=get_caps(handle).NumberInputValueCaps;
		HIDP_BUTTON_CAPS* ptr=new HIDP_BUTTON_CAPS[size];

		if(HidP_GetButtonCaps(HidP_Input,ptr,&size,(PHIDP_PREPARSED_DATA)preparsed_data.data())
			!=HIDP_STATUS_SUCCESS)
			return {};

		for(unsigned int ii=0;ii<size;++ii)
			caps.push_back(ptr[ii]);

		delete[](ptr);
	}
	catch(...)
	{}

	return caps;
}

static bool update_axes(const HANDLE& handle,const std::string& report,std::vector<float>& axes)
{
	preparsed_data_t preparsed_data=get_preparsed_data(handle);
	auto caps=get_axis_caps(handle);
	auto axes_temp=axes;
	unsigned int axes_index=0;

	for(unsigned int ii=0;ii<caps.size();++ii)
	{
		unsigned long value=0;

		if(caps[ii].ReportID==0x01&&HidP_GetUsageValue(HidP_Input,caps[ii].UsagePage,0,caps[ii].NotRange.Usage,&value,
			(PHIDP_PREPARSED_DATA)preparsed_data.c_str(),(char*)report.c_str(),
			report.size())!=HIDP_STATUS_SUCCESS)
			return false;

		axes_temp[axes_index]=value*2.0/(float)(caps[ii].LogicalMax-caps[ii].LogicalMin)-1.0;
		++axes_index;

		if(caps[ii].NotRange.Usage==HID_USAGE_HAT)
		{
			axes_temp[axes_index-1]=axes_temp[axes_index]=0.0;

			if(value>=0&&value<=7)
			{
				if(value>4) axes_temp[axes_index-1]=-1.0;
				if(value>0&&value<4) axes_temp[axes_index-1]=1.0;
				if(value<2||value>6) axes_temp[axes_index]=-1.0;
				if(value>2&&value<6) axes_temp[axes_index]=1.0;
			}

			++axes_index;
		}
	}

	axes=axes_temp;
	return true;
}

static bool update_buttons(const HANDLE& handle,const std::string& report,std::vector<bool>& buttons)
{
	preparsed_data_t preparsed_data=get_preparsed_data(handle);
	auto caps=get_button_caps(handle);
	std::vector<bool> buttons_temp=buttons;

	for(auto ii:buttons_temp)
		ii=false;

	for(unsigned int ii=0;ii<caps.size();++ii)
	{
		unsigned long buttons_count=caps[ii].Range.UsageMax-caps[ii].Range.UsageMin+1;
		USAGE usages[buttons_count];

		if(HidP_GetUsages(HidP_Input,caps[ii].UsagePage,0,usages,&buttons_count,(PHIDP_PREPARSED_DATA)preparsed_data.c_str(),(char*)report.c_str(),report.size())==HIDP_STATUS_SUCCESS)
			for(unsigned int bb=0;bb<buttons_count;++bb)
				buttons_temp[usages[bb]-caps[ii].Range.UsageMin]=true;
	}

	buttons=buttons_temp;
	return true;
}

std::vector<msl::js_info_t> msl::joystick_t::list()
{
	static std::vector<msl::js_info_t> list;

	for(auto ii:hid_list(HID_USAGE_JOYSTICK))
		list.push_back({hid_get_name(ii),ii});

	return list;
}

static bool joystick_valid_fd(const msl::js_fd_t& fd)
{
	return (fd.handle!=INVALID_HANDLE_VALUE);
}

static msl::js_fd_t joystick_open(const msl::js_info_t info)
{
	return {CreateFile(info.name.c_str(),GENERIC_READ,0,nullptr,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr),info.device};
}

static bool joystick_close(const msl::js_fd_t& fd)
{
	bool ret=CloseHandle(fd.handle);
	fd_m.handle=INVALID_HANDLE_VALUE;
	return ret;
}

static void joystick_update(msl::js_fd_t& fd,std::vector<float>& axes,std::vector<bool>& buttons)
{
	if(joystick_valid_fd(fd))
	{
		std::string report;
		report.resize(get_caps(fd.device.hDevice).InputReportByteLength);
		DWORD bytes_read=0;

		if(ReadFile(fd.handle,(char*)report.c_str(),report.size(),&bytes_read,nullptr)!=0)
		{
			if(bytes_read==report.size()&&report[0]==0x01)
			{
				update_axes(fd.device.hDevice,report,axes);
				update_buttons(fd.device.hDevice,report,buttons);
			}
		}
		else
		{
			fd.handle=INVALID_HANDLE_VALUE;
		}
	}
}

static size_t joystick_axis_count(const msl::js_fd_t& fd)
{
	auto caps=get_axis_caps(fd.device.hDevice);
	size_t count=0;

	for(auto ii:caps)
		if(ii.ReportID==0x01)
		{
			if(ii.NotRange.Usage==HID_USAGE_HAT)
				count+=2;
			else
				count+=ii.Range.UsageMax-ii.Range.UsageMin+1;
		}

	return count;
}

static size_t joystick_button_count(const msl::js_fd_t& fd)
{
	auto caps=get_button_caps(fd.device.hDevice);
	size_t count=0;

	for(auto ii:caps)
		if(ii.ReportID==0x01)
			count+=ii.Range.UsageMax-ii.Range.UsageMin+1;

	return count;
}































#elif(defined(__APPLE__))

#include <map>
#include <iostream>

static std::map<IOHIDDeviceRef,bool> device_allocated;

class hid_device_t
{
	public:
		IOHIDDeviceRef device;
		CFArrayRef elements;
		std::string name;
		std::vector<float> axes;
		std::vector<int> hats;
		std::vector<bool> buttons;

		enum type_t
		{
			NONE,
			AXIS,
			HAT,
			BUTTON
		};

		hid_device_t(const IOHIDDeviceRef& device,const bool init=false);
		bool operator==(const IOHIDDeviceRef& rhs) const;
		void initialize();
		void release();
		void update();
};

hid_device_t::hid_device_t(const IOHIDDeviceRef& device,const bool init):
	device(device),elements(nullptr),name("")
{
	if(device&&device_allocated.count(device)==0)
		device_allocated[device]=false;
	if(init)
		initialize();
}

bool hid_device_t::operator==(const IOHIDDeviceRef& rhs) const
{
	return device==rhs;
}

void hid_device_t::initialize()
{
	name=CFStringGetCStringPtr(
		(CFStringRef)IOHIDDeviceGetProperty(device,
		CFSTR(kIOHIDProductKey)),kCFStringEncodingASCII);
	elements=IOHIDDeviceCopyMatchingElements(device,nullptr,kIOHIDOptionsTypeNone);
	device_allocated[device]=true;
}

void hid_device_t::release()
{
	CFRelease(elements);
	elements=nullptr;
	if(device&&device_allocated[device])
	{
		IOHIDDeviceClose(device,kIOHIDOptionsTypeSeizeDevice);
		CFRelease(device);
		device_allocated[device]=false;
	}
	device=nullptr;
}

void hid_device_t::update()
{
	if(!device)
		return;
	if(device&&elements)
	{
		std::vector<float> axes_temp;
		std::vector<int> hats_temp;
		std::vector<bool> buttons_temp;
		if(!elements)
			return;
		for(size_t ii=0;ii<CFArrayGetCount(elements);++ii)
		{
			IOHIDElementRef element=(IOHIDElementRef)CFArrayGetValueAtIndex(elements,ii);
			int usage_page=IOHIDElementGetUsagePage(element);
			int usage=IOHIDElementGetUsage(element);
			type_t type=NONE;
			if(usage_page==kHIDPage_GenericDesktop)
			{
				if(usage==kHIDUsage_GD_X||usage==kHIDUsage_GD_Y||usage==kHIDUsage_GD_Z||
					usage==kHIDUsage_GD_Rx||usage==kHIDUsage_GD_Ry||usage==kHIDUsage_GD_Rz||
					usage==kHIDUsage_GD_Slider||usage==kHIDUsage_GD_Dial||usage==kHIDUsage_GD_Wheel)
					type=AXIS;
				else if(usage==kHIDUsage_GD_Hatswitch)
					type=HAT;
			}
			else if(usage_page==kHIDPage_Button)
			{
				type=BUTTON;
			}
			if(type!=NONE)
			{
				int min=IOHIDElementGetLogicalMin(element);
				int max=IOHIDElementGetLogicalMax(element);
				IOHIDValueRef new_value;
				int value=0;
				if(value<min)
					value=min;
				if(value>max)
					value=max;
				if(IOHIDDeviceGetValue(device,element,&new_value)==kIOReturnSuccess)
					value=IOHIDValueGetIntegerValue(new_value);
				if(type==AXIS)
				{
					float val=(value*2.0-max+min)/(max-min);
					if(val<-1)
						val=-1;
					if(val>1)
						val=1;
					axes_temp.push_back(val);
				}
				else if(type==HAT)
					hats_temp.push_back(value);
				else if(type==BUTTON)
					buttons_temp.push_back(value);
			}
		}
		if(axes.size()<axes_temp.size())
			axes.resize(axes_temp.size());
		for(size_t ii=0;ii<axes_temp.size();++ii)
			axes[ii]=axes_temp[ii];
		if(hats.size()<hats_temp.size())
			hats.resize(hats_temp.size());
		for(size_t ii=0;ii<hats_temp.size();++ii)
			hats[ii]=hats_temp[ii];
		if(buttons.size()<buttons_temp.size())
			buttons.resize(buttons_temp.size());
		for(size_t ii=0;ii<buttons_temp.size();++ii)
			buttons[ii]=buttons_temp[ii];
	}
}

static void add_cb(void* context,IOReturn result,void* sender,IOHIDDeviceRef device);
static void remove_cb(void* context,IOReturn result,void* sender,IOHIDDeviceRef device);

class hid_manager_t
{
	public:
		IOHIDManagerRef manager;
		std::vector<hid_device_t> devices;
		std::mutex mutex;

		hid_manager_t()
		{
			std::thread thread(std::bind(&hid_manager_t::update,this));
			thread.detach();
		}

		~hid_manager_t()
		{
			mutex.lock();
			for(auto& device:devices)
				device.release();
			mutex.unlock();
			IOHIDManagerClose(manager,kIOHIDOptionsTypeNone);
			CFRelease(manager);
		}

		hid_manager_t(const hid_manager_t& copy)=delete;
		hid_manager_t& operator=(const hid_manager_t& copy)=delete;

		void update()
		{
			manager=IOHIDManagerCreate(kCFAllocatorDefault,kIOHIDOptionsTypeNone);
			if(!manager)
				throw std::runtime_error("Error creating HID manager.");
			IOHIDManagerOpen(manager,kIOHIDOptionsTypeNone);
			IOHIDManagerSetDeviceMatchingMultiple(manager,create_dictionary());
			IOHIDManagerRegisterDeviceMatchingCallback(manager,add_cb,this);
			IOHIDManagerRegisterDeviceRemovalCallback(manager,remove_cb,this);
			IOHIDManagerScheduleWithRunLoop(manager,CFRunLoopGetCurrent(),CFSTR("MSL"));
			CFRunLoopRunInMode(CFSTR("MSL"),0,true);

			while(true)
			{
				CFRunLoopRunInMode(CFSTR("MSL"),0,true);
				mutex.lock();
				for(auto& device:devices)
					device.update();
				mutex.unlock();
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}
		}

		std::vector<hid_device_t> get_devices()
		{
			std::vector<hid_device_t> good_devices;
			mutex.lock();
			for(auto& device:devices)
				if(device.device!=nullptr&&device_allocated.count(device.device)>0&&
					device_allocated[device.device])
					good_devices.push_back(device);
			mutex.unlock();
			return good_devices;
		}

		bool device_ok(IOHIDDeviceRef find_device)
		{
			mutex.lock();
			bool ok=(find_device!=nullptr&&device_allocated.count(find_device)>0&&
				device_allocated[find_device]);
			if(ok)
			{
				ok=false;
				for(auto& device:devices)
					if(device==find_device)
					{
						ok=true;
						break;
					}
			}
			mutex.unlock();
			return ok;
		}

		void initialize_device(IOHIDDeviceRef new_device)
		{
			mutex.lock();
			bool found=false;
			for(auto& device:devices)
				if(device==new_device)
				{
					device.initialize();
					found=true;
					break;
				}
			if(!found)
				devices.push_back({new_device,true});
			mutex.unlock();
		}

		void remove_device(IOHIDDeviceRef old_device)
		{
			mutex.lock();
			for(size_t ii=0;ii<devices.size();++ii)
				if(devices[ii]==old_device)
				{
					devices[ii].release();
					devices.erase(devices.begin()+ii);
					break;
				}
			mutex.unlock();
		}

		void get_update(IOHIDDeviceRef find_device,std::vector<float>& axes,std::vector<bool>& buttons)
		{
			mutex.lock();
			for(auto& device:devices)
				if(device==find_device)
				{
					axes=device.axes;
					buttons=device.buttons;
					break;
				}
			mutex.unlock();
		}

		size_t get_axes_count(IOHIDDeviceRef find_device)
		{
			size_t count=0;
			mutex.lock();
			for(auto& device:devices)
				if(device==find_device)
				{
					count=device.axes.size();
					break;
				}
			mutex.unlock();
			return count;
		}

		size_t get_buttons_count(IOHIDDeviceRef find_device)
		{
			size_t count=0;
			mutex.lock();
			for(auto& device:devices)
				if(device==find_device)
				{
					count=device.buttons.size();
					break;
				}
			mutex.unlock();
			return count;
		}

		void* create_match(const uint32_t usage_page,const uint32_t usage,bool& error)
		{
			error=true;
			CFDictionaryRef dictionary=NULL;
			CFNumberRef usage_page_index=CFNumberCreate(kCFAllocatorDefault,kCFNumberIntType,&usage_page);
			CFNumberRef usage_index=CFNumberCreate(kCFAllocatorDefault,kCFNumberIntType,&usage);
			std::vector<void*> keys
			{
				(void*)CFSTR(kIOHIDDeviceUsagePageKey),
				(void*)CFSTR(kIOHIDDeviceUsageKey)
			};
			std::vector<void*> vals
			{
				(void*)usage_page_index,
				(void*)usage_index
			};
			if(usage_page_index&&usage_index)
				dictionary=CFDictionaryCreate(kCFAllocatorDefault,(const void**)keys.data(),(const void**)vals.data(),
					keys.size(),&kCFTypeDictionaryKeyCallBacks,&kCFTypeDictionaryValueCallBacks);
			if(usage_page_index)
				CFRelease(usage_page_index);
			if(usage_index)
				CFRelease(usage_index);
			if(dictionary!=nullptr)
				error=false;
			return (void*)dictionary;
		}

		CFArrayRef create_dictionary()
		{
			bool error=false;
			std::vector<void*> matches;
			matches.push_back(create_match(kHIDPage_GenericDesktop,kHIDUsage_GD_Joystick,error));
			matches.push_back(create_match(kHIDPage_GenericDesktop,kHIDUsage_GD_GamePad,error));
			matches.push_back(create_match(kHIDPage_GenericDesktop,kHIDUsage_GD_MultiAxisController,error));
			if(error)
				throw std::runtime_error("Error building HID dictionary.");
			CFArrayRef dictionary=CFArrayCreate(kCFAllocatorDefault,(const void**)matches.data(),matches.size(),
				&kCFTypeArrayCallBacks);
			for(auto match:matches)
				if(match)
					CFRelease((CFTypeRef*)match);
			return dictionary;
		}
};

static hid_manager_t hid_manager;

static void add_cb(void* context,IOReturn result,void* sender,IOHIDDeviceRef device)
{
	if(result!=kIOReturnSuccess)
		return;
	hid_manager_t& manager=*(hid_manager_t*)context;
	manager.initialize_device(device);
}

static void remove_cb(void* context,IOReturn result,void* sender,IOHIDDeviceRef device)
{
	hid_manager_t& manager=*(hid_manager_t*)context;
	manager.remove_device(device);
}








std::vector<msl::js_info_t> msl::joystick_t::list()
{
	std::vector<msl::js_info_t> list;
	auto devices=hid_manager.get_devices();
	for(auto device:devices)
		list.push_back({device.name,device.device});
	return list;
}

static bool joystick_valid_fd(const msl::js_fd_t& fd)
{
	return hid_manager.device_ok(fd.device);
}

static msl::js_fd_t joystick_open(const msl::js_info_t info)
{
	return {info.device};
}

static bool joystick_close(const msl::js_fd_t& fd)
{
	return true;
}

static void joystick_update(msl::js_fd_t& fd,std::vector<float>& axes,std::vector<bool>& buttons)
{
	hid_manager.get_update(fd.device,axes,buttons);
}

static size_t joystick_axis_count(const msl::js_fd_t& fd)
{
	return hid_manager.get_axes_count(fd.device);
}

static size_t joystick_button_count(const msl::js_fd_t& fd)
{
	return hid_manager.get_buttons_count(fd.device);
}

#else











































#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <unistd.h>

#define INVALID_HANDLE_VALUE (-1)

std::vector<msl::js_info_t> msl::joystick_t::list()
{
	std::vector<msl::js_info_t> list;
	std::vector<std::string> files;

	DIR* dp=opendir("/dev/input");

	while(dp!=nullptr)
	{
		dirent* np=readdir(dp);

		if(np==nullptr)
		{
			closedir(dp);
			break;
		}

		std::string node_name(np->d_name);

		if(node_name!="."&&node_name!=".."&&np->d_type==DT_CHR)
			files.push_back(node_name);
	}

	for(auto ii:files)
		if(ii.find("js")==0)
			list.push_back({"/dev/input/"+ii});

	return list;
}

static bool joystick_valid_fd(const msl::js_fd_t& fd)
{
	return (fd.handle!=INVALID_HANDLE_VALUE);
}

static msl::js_fd_t joystick_open(const msl::js_info_t info)
{
	return {open(info.name.c_str(),O_RDONLY)};
}

static bool joystick_close(const msl::js_fd_t& fd)
{
	int ret=close(fd.handle);
	fd_m.handle=INVALID_HANDLE_VALUE;
	return ret;
}

static void joystick_update(msl::js_fd_t& fd,std::vector<float>& axes,std::vector<bool>& buttons)
{
	if(joystick_valid_fd(fd))
	{
		js_event js;

		if(read(fd.handle,&js,sizeof(js_event))==sizeof(js_event))
		{
			unsigned char event=js.type&(~JS_EVENT_INIT);

			if(event==JS_EVENT_AXIS)
				axes[js.number]=js.value/32767.0;
			if(event==JS_EVENT_BUTTON)
				buttons[js.number]=js.value;
		}
		else
		{
			fd.handle=INVALID_HANDLE_VALUE;
		}
	}
}

static size_t joystick_axis_count(const msl::js_fd_t& fd)
{
	unsigned char count=0;
	ioctl(fd.handle,JSIOCGAXES,&count);
	return (size_t)count;
}

static size_t joystick_button_count(const msl::js_fd_t& fd)
{
	unsigned char count=0;
	ioctl(fd.handle,JSIOCGBUTTONS,&count);
	return (size_t)count;
}

#endif

msl::joystick_t::joystick_t(const msl::js_info_t info):info_m(info),axes_m({}),buttons_m({})
{}

msl::joystick_t::~joystick_t()
{
	close();
}

void msl::joystick_t::open()
{
	lock_m.lock();
	fd_m=joystick_open(info_m);
	lock_m.unlock();

	if(good())
	{
		lock_m.lock();
		axes_m.resize(joystick_axis_count(fd_m));
		buttons_m.resize(joystick_button_count(fd_m));
		std::thread thread(std::bind(&msl::joystick_t::update_m,this));
		thread.detach();
		lock_m.unlock();
	}
}

void msl::joystick_t::close()
{
	lock_m.lock();
	joystick_close(fd_m);
	lock_m.unlock();
}

bool msl::joystick_t::good()
{
	lock_m.lock();
	auto value=joystick_valid_fd(fd_m);
	lock_m.unlock();
	return value;
}

msl::js_info_t msl::joystick_t::info() const
{
	return info_m;
}

float msl::joystick_t::axis(const size_t index)
{
	if(index>=axis_count())
		throw std::out_of_range("joystick_t::axis");

	lock_m.lock();
	auto value=axes_m[index];
	lock_m.unlock();
	return value;
}

bool msl::joystick_t::button(const size_t index)
{
	if(index>=button_count())
		throw std::out_of_range("joystick_t::button");

	lock_m.lock();
	auto value=buttons_m[index];
	lock_m.unlock();
	return value;
}

size_t msl::joystick_t::axis_count()
{
	lock_m.lock();
	auto count=axes_m.size();
	lock_m.unlock();
	return count;
}

size_t msl::joystick_t::button_count()
{
	lock_m.lock();
	auto count=buttons_m.size();
	lock_m.unlock();
	return count;
}

void msl::joystick_t::update_m()
{
	while(true)
	{
		try
		{
			while(good())
			{
				lock_m.lock();
				auto fd_copy=fd_m;
				auto axes_copy=axes_m;
				auto buttons_copy=buttons_m;

				joystick_update(fd_copy,axes_copy,buttons_copy);

				fd_m=fd_copy;
				axes_m=axes_copy;
				buttons_m=buttons_copy;
				lock_m.unlock();

				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}
			break;
		}
		catch(...)
		{
		}
	}
}