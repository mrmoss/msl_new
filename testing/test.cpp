#include <iostream>
#include "../socket.hpp"

int main()
{
	msl::socket c("0.0.0.0:255<0.0.0.0:0",false);
	c.open();

	if(c.good())
	{
		std::cout<<":("<<std::endl;
		return 0;
	}

	std::cout<<":)\t"<<c.address()<<std::endl;

	return 0;
}