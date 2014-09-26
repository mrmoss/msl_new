#include <iostream>
#include "serial.hpp"
#include "time.hpp"

int main()
{
	serial_fd_t fd=serial_open("/dev/ttyUSB0",57600);
	msl::delay_ms(2000);

	serial_write(fd,"a",1);

	std::cout<<"reading"<<std::endl;
	while(serial_available(fd)>=0)
	{
		char temp;

		while(serial_available(fd)>0&&serial_read(fd,&temp,1)==1)
			std::cout<<temp<<std::flush;
	}

	serial_close(fd);

	return 0;
}