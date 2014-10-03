#include <iostream>
#include "../serial.hpp"
#include "../time.hpp"

int main()
{
	while(true)
	{
		auto list=msl::serial::list();

		std::cout<<"Found "<<list.size()<<" serials."<<std::endl;

		if(list.size()>0)
		{
			msl::serial js(list[0],57600);
			js.open();

			if(!js.good())
				std::cout<<"Failed to open serial 0."<<std::endl;

			while(js.good())
			{
				/*std::cout<<js.axis(0)<<std::endl;*/
				auto list=msl::serial::list();
				std::cout<<list.size()<<std::endl;
				msl::delay_ms(1);
			}

			std::cout<<"Lost connection to serial 0."<<std::endl;
		}

		msl::delay_ms(1);
	}

	return 0;
}
