#include <iostream>
#include "joystick.hpp"
#include "time.hpp"

int main()
{
	while(true)
	{
		auto list=msl::joystick::list();

		std::cout<<"Found "<<list.size()<<" joysticks."<<std::endl;

		if(list.size()>0)
		{
			msl::joystick js(list[0]);
			js.open();

			if(!js.good())
				std::cout<<"Failed to open joystick 0."<<std::endl;

			while(js.good())
			{
				std::cout<<js.axis(0)<<std::endl;
				msl::delay_ms(1);
			}

			std::cout<<"Lost connection to joystick 0."<<std::endl;
		}

		msl::delay_ms(1);
	}

	return 0;
}
