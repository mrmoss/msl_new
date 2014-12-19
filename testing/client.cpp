#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include "../socket.hpp"
#include "../time.hpp"

int main()
{
	uint8_t test[4]={127,0,0,1};
	uint16_t port=8080;

	msl::socket c(test,port,false);
	c.open();

	if(!c.good())
	{
		std::cout<<":("<<std::endl;
		return 0;
	}

	std::cout<<":)"<<std::endl;

	std::string message="";

	auto timer=msl::millis()+1000;


	while(c.good())
	{
		if(msl::millis()>=timer)
		{
			std::string data="hello server!\n";
			c.write(data.c_str(),data.size());
			timer=msl::millis()+1000;
		}

		char temp;

		while(c.available()>0&&c.read(&temp,1)==1)
		{
			if(temp=='\n')
			{
				std::cout<<"server said \""<<message<<"\"."<<std::endl;
				message="";
			}
			else
			{
				message+=temp;
			}
		}
	}

	std::cout<<"disconnected from server..."<<std::endl;

	return 0;
}