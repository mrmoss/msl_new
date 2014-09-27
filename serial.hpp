#ifndef MSL_2011_SERIAL_HPP
#define MSL_2011_SERIAL_HPP

#include <string>

#if(defined(_WIN32)&&!defined(__CYGWIN__))
#include <conio.h>
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#endif

namespace msl
{

	#if(defined(_WIN32)&&!defined(__CYGWIN__))
	typedef HANDLE serial_fd_t;
	#else
	typedef ssize_t serial_fd_t;
	#endif

	struct serial_device_t
	{
		msl::serial_fd_t fd;
		std::string name;
		size_t baud;
	};

	serial_device_t serial_open(const std::string& name,const size_t baud);
	void serial_close(const serial_device_t& device);
	bool serial_valid(const serial_device_t& device);
	ssize_t serial_available(const serial_device_t& device);
	ssize_t serial_read(const serial_device_t& device,void* buffer,const size_t size);
	ssize_t serial_write(const serial_device_t& device,const void* buffer,const size_t size);

	class serial
	{
		public:
			serial(const std::string& name,const size_t baud);
			void open();
			void close();
			bool good() const;
			ssize_t available() const;
			ssize_t read(void* buf,const size_t count) const;
			ssize_t write(const void* buf,const size_t count) const;

		private:
			serial_device_t device_m;
	};
}

#endif