#include "socket.hpp"

#include <string.h>

#define INVALID_SOCKET_VALUE (~0)

#if(!defined(_WIN32)||defined(__CYGWIN__))
	#include <signal.h>
#else
	int close(unsigned int fd)
	{
		return closesocket(fd);
	}
#endif

static bool socket_inited=false;

static void socket_init()
{
	if(!socket_inited)
	{
		socket_inited=true;

		#if(defined(_WIN32)&&!defined(__CYGWIN__))
			WSADATA temp;
			WSAStartup(0x0002,&temp);
		#else
			signal(SIGPIPE,SIG_IGN);
		#endif
	}
}

static void socket_close(msl::socket_device_t& device)
{
	close(device.fd);
}

static void socket_open(msl::socket_device_t& device)
{
	socket_init();

	int type=SOCK_STREAM;

	if(!device.tcp)
		type=SOCK_DGRAM;

	device.fd=socket(AF_INET,type,0);

	if(device.host)
	{
		linger lingerer;
		lingerer.l_onoff=1;
		lingerer.l_linger=10;
		int on=1;
		socklen_t address_length=sizeof(device.address);

		if(device.fd!=INVALID_SOCKET_VALUE)
		{
			if(setsockopt(device.fd,SOL_SOCKET,SO_LINGER,(const char*)&lingerer,sizeof(lingerer))!=0)
				socket_close(device);

			if(setsockopt(device.fd,SOL_SOCKET,SO_REUSEADDR,(const char*)&on,sizeof(on))!=0)
				socket_close(device);

			if(!device.tcp&&setsockopt(device.fd,SOL_SOCKET,SO_RCVBUF,(const char*)&device.buffer_size,address_length)!=0)
				socket_close(device);

			if(!device.tcp&&setsockopt(device.fd,SOL_SOCKET,SO_SNDBUF,(const char*)&device.buffer_size,address_length)!=0)
				socket_close(device);

			if(bind(device.fd,(sockaddr*)&device.address,sizeof(device.address))!=0)
				socket_close(device);

			if(device.tcp&&listen(device.fd,16))
				socket_close(device);
		}
	}
	else
	{
		if(connect(device.fd,(sockaddr*)&device.address,sizeof(device.address))!=0)
			device.fd=INVALID_SOCKET_VALUE;
	}
}

static ssize_t socket_available(const msl::socket_device_t& device)
{
	socket_init();

	if(device.fd==INVALID_SOCKET_VALUE)
		return -1;

	timeval temp={0,0};
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET((unsigned int)device.fd,&rfds);

	return select(1+device.fd,&rfds,nullptr,nullptr,&temp);
}

static ssize_t socket_read(const msl::socket_device_t& device,void* buffer,const size_t size,const int flags=0)
{
	socket_init();
	return recv(device.fd,(char*)buffer,size,flags);
}

static ssize_t socket_write(const msl::socket_device_t& device,const void* buffer,const size_t size,const int flags=0)
{
	socket_init();
	return send(device.fd,(char*)buffer,size,flags);
}

static bool socket_valid(const msl::socket_device_t& device)
{
	socket_init();

	if(device.fd==INVALID_SOCKET_VALUE)
		return false;

	char temp;

	if(socket_available(device)>0&&socket_read(device,&temp,1,MSG_PEEK)==0)
		return false;

	return (socket_available(device)>=0);
}

static msl::socket_device_t socket_accept(const msl::socket_device_t& device)
{
	socket_init();
	msl::socket_device_t client{INVALID_SOCKET_VALUE,{},false,device.tcp,device.buffer_size};

	if(socket_available(device)>0)
	{
		socklen_t address_length=sizeof(client.address);
		client.fd=accept(device.fd,(sockaddr*)&client.address,&address_length);
	}

	return client;
}

msl::socket::socket(const uint8_t* address,const uint16_t& port,bool host,const bool tcp)
{
	device_m.address.sin_family=AF_INET;
	memcpy(&device_m.address.sin_addr,address,4);
	device_m.address.sin_port=htons(port);
	device_m.host=host;
	device_m.tcp=tcp;
}

msl::socket::socket(const msl::socket_device_t& device):device_m(device)
{}

void msl::socket::open()
{
	socket_open(device_m);
}

void msl::socket::close()
{
	socket_close(device_m);
}

bool msl::socket::good() const
{
	return socket_valid(device_m);
}

ssize_t msl::socket::available() const
{
	return socket_available(device_m);
}

ssize_t msl::socket::read(void* buf,const size_t count) const
{
	return socket_read(device_m,buf,count);
}

ssize_t msl::socket::write(const void* buf,const size_t count) const
{
	return socket_write(device_m,buf,count);
}

ssize_t msl::socket::write(const std::string& buf) const
{
	return socket_write(device_m,buf.c_str(),buf.size());
}

msl::socket msl::socket::accept()
{
	return msl::socket(socket_accept(device_m));
}