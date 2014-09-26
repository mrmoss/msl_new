#include "serial.hpp"

static bool valid_baud(const uint32_t baud)
{
	return (baud==300||baud==1200||baud==2400||baud==4800||baud==9600||
		baud==19200||baud==38400||baud==57600||baud==115200);
}

#if(defined(_WIN32)&&!defined(__CYGWIN__))

static SSIZE_T read(serial_fd_t fd,void* buf,size_t count)
{
	DWORD bytes_read=-1;
	if(ReadFile(fd,buf,count,&bytes_read,0))
		return bytes_read;
	return -1;
}
static SSIZE_T write(serial_fd_t fd,void* buf,size_t count)
{
	DWORD bytes_sent=-1;
	if(WriteFile(fd,buf,count,&bytes_sent,0))
		return bytes_sent;
	return -1;
}

int select(serial_fd_t nfds)
{
	COMSTAT port_stats;
	DWORD error_flags=0;
	if(ClearCommError(nfds,&error_flags,&port_stats))
		return port_stats.cbInQue;
	return -1;
}

serial_fd_t serial_open(const std::string& name,const uint32_t baud)
{
	std::string full_path="\\\\.\\"+name;
	HANDLE fd=CreateFile(full_path.c_str(),GENERIC_READ|GENERIC_WRITE,0,0,CREATE_ALWAYS,0,nullptr);
	DCB options;

	if(fd!=INVALID_HANDLE_VALUE&&valid_baud(baud)&&GetCommState(fd,&options))
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
	}

	if(SetCommState(fd,&options)&&EscapeCommFunction(fd,CLRDTR|CLRRTS))
		return fd;

	serial_close(fd);
	return INVALID_HANDLE_VALUE;
}

void serial_close(const serial_fd_t fd)
{
	CloseHandle(fd);
}

#else

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

#define INVALID_HANDLE_VALUE (-1)

int select(serial_fd_t nfds)
{
	timeval temp={0,0};
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(nfds,&rfds);
	return select(nfds+1,&rfds,nullptr,nullptr,&temp);
}

static speed_t baud_rate(const uint32_t baud)
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

serial_fd_t serial_open(const std::string& name,const uint32_t baud)
{
	serial_fd_t fd=open(name.c_str(),O_RDWR|O_NOCTTY|O_SYNC);
	termios options;

	if(fd!=INVALID_HANDLE_VALUE&&valid_baud(baud)&&tcgetattr(fd,&options)!=-1&&
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
	}

	if(tcsetattr(fd,TCSANOW,&options)!=-1&&tcflush(fd,TCIFLUSH)!=-1&&tcdrain(fd)!=-1)
		return fd;

	serial_close(fd);
	return INVALID_HANDLE_VALUE;
}

void serial_close(const serial_fd_t fd)
{
	close(fd);
}

#endif

int serial_available(const serial_fd_t fd)
{
	if(fd==INVALID_HANDLE_VALUE)
		return -1;

	int return_value=-1;

	return_value=select(fd);

	//if(return_value>0)
		//break;

	return return_value;
}

int serial_read(const serial_fd_t fd,void* buffer,const size_t size)
{
	if(fd==INVALID_HANDLE_VALUE)
		return -1;

	unsigned int bytes_unread=size;
	unsigned int bytes_read=read(fd,(char*)buffer+(size-bytes_unread),bytes_unread);

	if(bytes_read>0)
	{
		bytes_unread-=bytes_read;

		if(bytes_unread==0)
			return size;
	}

	return (size-bytes_unread);
}

int serial_write(const serial_fd_t fd,const void* buffer,const size_t size)
{
	if(fd==INVALID_HANDLE_VALUE)
		return -1;

	unsigned int bytes_unsent=size;
	unsigned int bytes_sent=write(fd,(char*)buffer+(size-bytes_unsent),bytes_unsent);

	if(bytes_sent>0)
	{
		bytes_unsent-=bytes_sent;

		if(bytes_unsent==0)
			return size;
	}

	return (size-bytes_unsent);
}