#include <iostream>
#include <string>
#include "../serial.hpp"
#include "../time.hpp"

int main()
{
	while(true)
	{
		msl::serial_t serial("/dev/ttyUSB0",57600);
		serial.open();

		if(serial.good())
			std::cout<<":)"<<std::endl;

		while(serial.good())
		{
			uint8_t temp;

			while(serial.available()>0&&serial.read(&temp,1)==1)
				std::cout<<temp<<std::flush;

			msl::delay_ms(1);
		}

		std::cout<<":("<<std::endl;
		msl::delay_ms(100);
	}

	return 0;
}