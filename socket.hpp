#ifndef MSL_C11_SOCKET_HPP
#define MSL_C11_SOCKET_HPP

#include <string>
#include <vector>

#if(defined(_WIN32)&&!defined(__CYGWIN__))
	#include <winsock2.h>
	#if(!defined(socklen_t))
		typedef int socklen_t;
	#endif
#else
	#include <netinet/in.h>
	#include <unistd.h>
	#include <fcntl.h>
#endif

namespace msl
{

	#if(defined(_WIN32)&&!defined(__CYGWIN__))
		typedef HANDLE socket_fd_t;
	#else
		typedef int socket_fd_t;
	#endif

	struct socket_device_t
	{
		socket_fd_t fd;
		sockaddr_in address;
		bool host;
		bool tcp;
		int buffer_size;
	};

	class socket
	{
		public:
			socket(const uint8_t* address,const uint16_t& port,bool host,const bool tcp=true);
			socket(const socket_device_t& device);
			void open();
			void close();
			bool good() const;
			ssize_t available() const;
			ssize_t read(void* buf,const size_t count) const;
			ssize_t write(const void* buf,const size_t count) const;
			ssize_t write(const std::string& buf) const;
			socket accept();

		private:
			socket_device_t device_m;
	};
}

#endif