#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include "../socket.hpp"
#include "../time.hpp"

int main()
{
	msl::udp_socket c("127.0.0.1:8080",2048);
	c.open();

	if(!c.good())
	{
		std::cout<<":("<<std::endl;
		return 0;
	}

	std::cout<<":)"<<std::endl;

	auto timer=msl::millis()+1000;


	while(c.good())
	{
		if(msl::millis()>=timer)
		{
			std::cout<<"sending packet!"<<std::endl;
			std::string data;
			for(int ii=0;ii<2048;++ii)
				data+='a';
			c.write(data);
			timer=msl::millis()+1000;
		}

		/*char temp[2048];

		while(c.available()>0&&c.read(temp,2048)==2048)
			std::cout<<"received packet!"<<std::endl;*/

		msl::delay_ms(1);
	}

	std::cout<<"disconnected from server..."<<std::endl;

	return 0;
}