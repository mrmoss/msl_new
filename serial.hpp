#ifndef MSL_2011_SERIAL_HPP
#define MSL_2011_SERIAL_HPP

#include <cstdint>
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
	typedef int serial_fd_t;
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
	int serial_available(const serial_device_t& device);
	int serial_read(const serial_device_t& device,void* buffer,const size_t size);
	int serial_write(const serial_device_t& device,const void* buffer,const size_t size);
}

#endif