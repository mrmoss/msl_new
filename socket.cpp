//Windows Dependencies:
//		-lWs2_32

#include "socket.hpp"

#include <stdexcept>
#include <cstring>

#define INVALID_SOCKET_VALUE (~0)

#if(!defined(_WIN32)||defined(__CYGWIN__))
	#include <signal.h>
#else
	int close(unsigned int fd)
	{
		return closesocket(fd);
	}
#endif

void string_to_rawaddr(std::string str,uint8_t* ip,uint16_t& port)
{
	uint8_t temp_ip[4];
	uint16_t temp_port;

	bool saw_colon=false;
	int state=0;
	int dot_count=0;
	size_t start=0;
	bool finished=false;

	for(size_t ii=0;ii<str.size();++ii)
	{
		if(state==0&&isdigit(str[ii])!=0)
		{
			start=ii;

			if(dot_count==3&&saw_colon)
				state=3;
			else if(dot_count==3)
				state=2;
			else
				state=1;
		}
		else if(state==1)
		{

			if(isdigit(str[ii])==0)
			{
				if(str[ii]=='.')
				{
					++dot_count;

					if(dot_count>3)
					{
						state=-1;
					}
					else
					{
						auto check=std::stoi(str.substr(start,ii+1-start));

						if(check>=0&&check<=255)
						{
							temp_ip[dot_count-1]=check;
							state=0;
						}
						else
						{
							state=-1;
						}
					}
				}
				else
				{
					state=-1;
				}
			}
		}
		else if(state==2)
		{
			if(isdigit(str[ii])==0)
			{
				if(str[ii]==':')
				{
					if(saw_colon)
					{
						state=-1;
					}
					else
					{
						auto check=std::stoi(str.substr(start,ii+1-start));

						if(check>=0&&check<=255)
						{
							temp_ip[dot_count]=check;
							saw_colon=true;
							state=0;
						}
						else
						{
							state=-1;
						}
					}
				}
				else
				{
					state=-1;
				}
			}
		}
		else if(state==3)
		{
			if(isdigit(str[ii])==0)
			{
				state=-1;
			}
			else if(ii==str.size()-1)
			{
				auto check=std::stoi(str.substr(start,ii+1-start));

				if(check>=0&&check<=65535)
				{
					finished=true;
					temp_port=check;
				}
			}
		}
		else
		{
			break;
		}
	}

	if(!finished)
		throw std::runtime_error("void string_to_rawaddr(std::string str,uint8_t* ip,uint16_t& port) - Invalid ip string!");

	memcpy(ip,temp_ip,4);
	port=temp_port;
}

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
	socklen_t ip_length=sizeof(device.ip);

	if(device.fd!=INVALID_SOCKET_VALUE)
	{
		if(device.host)
		{
			linger lingerer;
			lingerer.l_onoff=1;
			lingerer.l_linger=10;
			int on=1;

			if(setsockopt(device.fd,SOL_SOCKET,SO_LINGER,(const char*)&lingerer,sizeof(lingerer))!=0)
				socket_close(device);
			if(setsockopt(device.fd,SOL_SOCKET,SO_REUSEADDR,(const char*)&on,sizeof(on))!=0)
				socket_close(device);

			if(bind(device.fd,(sockaddr*)&device.ip,sizeof(device.ip))!=0)
				socket_close(device);
			if(device.tcp&&listen(device.fd,16))
				socket_close(device);
			if(getsockname(device.fd,(sockaddr*)&device.ip,&ip_length))
				socket_close(device);
		}
		else
		{
			if(connect(device.fd,(sockaddr*)&device.ip,sizeof(device.ip))!=0)
				device.fd=INVALID_SOCKET_VALUE;

			if(!device.tcp&&setsockopt(device.fd,SOL_SOCKET,SO_RCVBUF,(const char*)&device.buffer_size,ip_length)!=0)
					socket_close(device);
			if(!device.tcp&&setsockopt(device.fd,SOL_SOCKET,SO_SNDBUF,(const char*)&device.buffer_size,ip_length)!=0)
				socket_close(device);
		}
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
		socklen_t ip_length=sizeof(client.ip);
		client.fd=accept(device.fd,(sockaddr*)&client.ip,&ip_length);
	}

	return client;
}

msl::socket::socket(const std::string& address,bool host,const bool tcp)
{
	uint8_t ip[4];
	uint16_t port;
	device_m.fd=INVALID_SOCKET_VALUE;
	string_to_rawaddr(address,ip,port);
	device_m.ip.sin_family=AF_INET;
	memcpy(&device_m.ip.sin_addr,ip,4);
	device_m.ip.sin_port=htons(port);
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
	device_m.fd=INVALID_SOCKET_VALUE;
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

msl::tcp_socket::tcp_socket(const std::string& address,bool host):msl::socket(address,host,true)
{}

msl::udp_socket::udp_socket(const std::string& address,const size_t buffer_size):
	msl::socket(address,false,false)
{
	device_m.buffer_size=buffer_size;
}