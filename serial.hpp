#ifndef MSL_C11_SERIAL_HPP
#define MSL_C11_SERIAL_HPP

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

	class serial
	{
		public:
			//static std::vector<js_info_t> list();

			serial(const std::string& name,const size_t baud);
			~serial();
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