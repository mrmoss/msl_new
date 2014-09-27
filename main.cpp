#include <iostream>
#include "serial.hpp"
#include "time.hpp"

int main()
{
	serial_device_t device=serial_open("/dev/ttyACM0",57600);

	if(serial_valid(device))
	{
		std::cout<<":)"<<std::endl;

		msl::delay_ms(2000);
		serial_write(device,"a",1);
		std::cout<<"reading"<<std::endl;

		while(serial_available(device)>=0)
		{
			char temp;

			while(serial_available(device)>0&&serial_read(device,&temp,1)==1)
				std::cout<<temp<<std::flush;
		}

		serial_close(device);
	}

	std::cout<<":("<<std::endl;

	return 0;
}
