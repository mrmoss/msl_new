#ifndef MSL_2011_SERIAL_HPP
#define MSL_2011_SERIAL_HPP

#include <cstdint>
#include <string>

#if(defined(_WIN32)&&!defined(__CYGWIN__))
#include <conio.h>
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
typedef HANDLE serial_fd_t;
#else
typedef int serial_fd_t;
#endif

serial_fd_t serial_open(const std::string& name,const unsigned int baud);
void serial_close(const serial_fd_t fd);
int serial_available(const serial_fd_t fd);
int serial_read(const serial_fd_t fd,void* buffer,const size_t size);
int serial_write(const serial_fd_t fd,const void* buffer,const size_t size);

#endif