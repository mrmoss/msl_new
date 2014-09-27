#include <iostream>
#include "serial.hpp"
#include "time.hpp"

int main()
{
	msl::serial_device_t device=msl::serial_open("/dev/ttyACM0",57600);

	if(msl::serial_valid(device))
	{
		std::cout<<":)"<<std::endl;

		msl::delay_ms(2000);
		msl::serial_write(device,"a",1);
		std::cout<<"reading"<<std::endl;

		while(msl::serial_available(device)>=0)
		{
			char temp;

			while(msl::serial_available(device)>0&&msl::serial_read(device,&temp,1)==1)
				std::cout<<temp<<std::flush;
		}

		msl::serial_close(device);
	}

	std::cout<<":("<<std::endl;

	return 0;
}
