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

			auto timer=msl::millis()+2000;

			while(test.good())
			{
				if(msl::millis()>=timer)
				{
					test.write("a",1);
					timer=msl::millis()+500;
				}

				char temp;

				while(test.available()>0&&test.read(&temp,1)==1)
					std::cout<<temp<<std::flush;

				msl::delay_ms(1);
			}

			test.close();
			msl::delay_ms(1);
		}

		std::cout<<":("<<std::endl;
		msl::delay_ms(1);
	}

	return 0;
}
