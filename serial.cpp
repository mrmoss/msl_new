#include "serial.hpp"

static bool valid_baud(const size_t baud)
{
	return (baud==300||baud==1200||baud==2400||baud==4800||baud==9600||
		baud==19200||baud==38400||baud==57600||baud==115200);
}

#if(defined(_WIN32)&&!defined(__CYGWIN__))

static ssize_t read(msl::serial_fd_t fd,void* buf,size_t count)
{
	DWORD bytes_read=-1;
	if(ReadFile(fd,buf,count,&bytes_read,0))
		return bytes_read;
	return -1;
}

static ssize_t write(msl::serial_fd_t fd,void* buf,size_t count)
{
	DWORD bytes_sent=-1;
	if(WriteFile(fd,buf,count,&bytes_sent,0))
		return bytes_sent;
	return -1;
}

static ssize_t select(msl::serial_device_t device)
{
	COMSTAT port_stats;
	DWORD error_flags=0;

	if(ClearCommError(device.fd,&error_flags,&port_stats))
		return port_stats.cbInQue;

	return -1;
}

static void serial_close(const msl::serial_device_t& device)
{
	CloseHandle(device.fd);
}

static msl::serial_device_t serial_open(const std::string& name,const size_t baud)
{
	msl::serial_device_t device{INVALID_HANDLE_VALUE,name,baud};
	std::string full_path="\\\\.\\"+device.name;
	device.fd=CreateFile(full_path.c_str(),GENERIC_READ|GENERIC_WRITE,
		0,0,OPEN_EXISTING,0,nullptr);
	DCB options;

	if(device.fd!=INVALID_HANDLE_VALUE&&valid_baud(baud)&&GetCommState(device.fd,&options))
	{
		options.BaudRate=baud;
		options.fParity=FALSE;
		options.Parity=NOPARITY;
		options.StopBits=ONESTOPBIT;
		options.ByteSize=8;
		options.fOutxCtsFlow=FALSE;
		options.fOutxDsrFlow=FALSE;
		options.fDtrControl=DTR_CONTROL_DISABLE;
		options.fRtsControl=RTS_CONTROL_DISABLE;

		if(SetCommState(device.fd,&options)&&EscapeCommFunction(device.fd,CLRDTR|CLRRTS))
			return device;
	}

	serial_close(device);
	device.fd=INVALID_HANDLE_VALUE;
	return device;
}

static bool serial_valid(const msl::serial_device_t& device)
{
	if(device.fd==INVALID_HANDLE_VALUE||!valid_baud(device.baud))
		return false;

	std::string full_path="\\\\.\\"+device.name;
	msl::serial_fd_t fd=CreateFile(full_path.c_str(),GENERIC_READ,0,0,OPEN_EXISTING,0,nullptr);
	return fd!=INVALID_HANDLE_VALUE||GetLastError()!=ERROR_FILE_NOT_FOUND;
}

#else

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

#define INVALID_HANDLE_VALUE (-1)

static ssize_t select(msl::serial_device_t device)
{
	timeval temp={0,0};
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(device.fd,&rfds);
	return select(device.fd+1,&rfds,nullptr,nullptr,&temp);
}

static speed_t baud_rate(const size_t baud)
{
	if(baud==300)
		return B300;
	else if(baud==1200)
		return B1200;
	else if(baud==2400)
		return B2400;
	else if(baud==4800)
		return B4800;
	else if(baud==9600)
		return B9600;
	else if(baud==19200)
		return B19200;
	else if(baud==38400)
		return B38400;
	else if(baud==57600)
		return B57600;
	else if(baud==115200)
		return B115200;

	return B0;
}

static void serial_close(const msl::serial_device_t& device)
{
	close(device.fd);
}

static msl::serial_device_t serial_open(const std::string& name,const size_t baud)
{
	msl::serial_device_t device{INVALID_HANDLE_VALUE,name,baud};

	device.fd=open(device.name.c_str(),O_RDWR|O_NOCTTY|O_SYNC);
	termios options;

	if(device.fd!=INVALID_HANDLE_VALUE&&valid_baud(baud)&&tcgetattr(device.fd,&options)!=-1&&
		cfsetispeed(&options,baud_rate(baud))!=-1&&cfsetospeed(&options,baud_rate(baud))!=-1)
	{
		options.c_cflag|=(CS8|CLOCAL|CREAD|HUPCL);
		options.c_iflag|=(IGNBRK|IGNPAR);
		options.c_iflag&=~(IXON|IXOFF|IXANY);
		options.c_lflag=0;
		options.c_oflag=0;
		options.c_cc[VMIN]=0;
		options.c_cc[VTIME]=1;
		options.c_cflag&=~(PARENB|PARODD);
		options.c_cflag&=~CSTOPB;
		options.c_cflag&=~CRTSCTS;

		if(tcsetattr(device.fd,TCSANOW,&options)!=-1&&tcflush(device.fd,TCIFLUSH)!=-1&&
			tcdrain(device.fd)!=-1)
			return device;
	}

	serial_close(device);
	device.fd=INVALID_HANDLE_VALUE;
	return device;
}

static bool serial_valid(const msl::serial_device_t& device)
{
	if(device.fd==INVALID_HANDLE_VALUE||!valid_baud(device.baud))
		return false;

	termios options;
	return tcgetattr(device.fd,&options)!=-1;
}

#endif

ssize_t serial_available(const msl::serial_device_t& device)
{
	if(!serial_valid(device))
		return -1;

	return select(device);
}

ssize_t serial_read(const msl::serial_device_t& device,void* buffer,const size_t size)
{
	if(!serial_valid(device))
		return -1;

	return read(device.fd,(char*)buffer,size);
}

ssize_t serial_write(const msl::serial_device_t& device,const void* buffer,const size_t size)
{
	if(!serial_valid(device))
		return -1;

	return write(device.fd,(char*)buffer,size);
}

msl::serial::serial(const std::string& name,const size_t baud):device_m{INVALID_HANDLE_VALUE,name,baud}
{}

msl::serial::~serial()
{
	close();
}

void msl::serial::open()
{
	device_m=serial_open(device_m.name,device_m.baud);
}

void msl::serial::close()
{
	serial_close(device_m);
}

bool msl::serial::good() const
{
	return serial_valid(device_m);
}

ssize_t msl::serial::available() const
{
	return serial_available(device_m);
}

ssize_t msl::serial::read(void* buf,const size_t count) const
{
	return serial_read(device_m,buf,count);
}

ssize_t msl::serial::write(const void* buf,const size_t count) const
{
	return serial_write(device_m,buf,count);
}