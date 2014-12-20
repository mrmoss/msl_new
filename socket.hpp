//Windows Dependencies:
//		-lWs2_32

#ifndef MSL_C11_SOCKET_HPP
#define MSL_C11_SOCKET_HPP

#include <cstdint>
#include <string>

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
	typedef int socket_fd_t;

	struct socket_device_t
	{
		socket_fd_t fd;
		sockaddr_in ip;
		bool host;
		bool tcp;
		size_t buffer_size;
	};

	class socket
	{
		public:
			socket(const uint8_t* ip,const uint16_t& port,bool host,const bool tcp=true);
			socket(const socket_device_t& device);
			socket(const std::string& address,bool host,const bool tcp=true);
			virtual void open();
			virtual void close();
			virtual bool good() const;
			virtual ssize_t available() const;
			virtual ssize_t read(void* buf,const size_t count) const;
			virtual ssize_t write(const void* buf,const size_t count) const;
			virtual ssize_t write(const std::string& buf) const;
			virtual socket accept();

		protected:
			socket_device_t device_m;
	};

	class tcp_socket:public socket
	{
		public:
			tcp_socket(const uint8_t* ip,const uint16_t& port,bool host);
			tcp_socket(const socket_device_t& device);
			tcp_socket(const std::string& address,bool host);
	};

	class udp_socket:public socket
	{
		public:
			udp_socket(const uint8_t* ip,const uint16_t& port,const size_t buffer_size);
			udp_socket(const socket_device_t& device,const size_t buffer_size);
			udp_socket(const std::string& address,const size_t buffer_size);
	};
}

#endif