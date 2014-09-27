#include <iostream>
#include "serial.hpp"
#include "time.hpp"

int main()
{
	while(true)
	{
		msl::serial test("/dev/ttyACM0",57600);
		test.open();

		if(test.good())
		{
			std::cout<<":)"<<std::endl;

			msl::delay_ms(2000);
			test.write("a",1);
			std::cout<<"reading"<<std::endl;

			while(test.good())
			{
				char temp;

				while(test.available()>0&&test.read(&temp,1)==1)
					std::cout<<temp<<std::flush;

				msl::delay_us(1);
			}

			test.close();
			msl::delay_us(1);
		}

		std::cout<<":("<<std::endl;
	}

	return 0;
}
