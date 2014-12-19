#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include "../socket.hpp"
#include "../time.hpp"

int main()
{
	msl::socket c("127.0.0.1:8080",false);
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
			c.write("hello server!\n");
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