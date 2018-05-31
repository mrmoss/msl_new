#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include "../socket.hpp"
#include "../time.hpp"

int main(int argc,char* argv[])
{
	const size_t buffer_size=200;
	char filler='a';
	if(std::string(argv[1])=="8081")
		filler='b';
	msl::udp_socket_t c("172.20.230.71:"+std::string(argv[1])+
		"<172.20.230.71:"+std::string(argv[2]),buffer_size);
	c.open();

	if(!c.good())
	{
		std::cout<<":("<<std::endl;
		return 0;
	}

	std::cout<<":)\t"<<c.address()<<std::endl;

	//auto timer=msl::millis()+1000;


	while(c.good())
	{
		/*if(msl::millis()>=timer)
		{
			std::cout<<"sending packet!"<<std::endl;
			std::string data;
			for(size_t ii=0;ii<buffer_size;++ii)
				data+=filler;
			c.write(data);
			timer=msl::millis()+1000;
		}*/

		char temp[buffer_size];
		//std::cout<<c.available()<<std::endl;

		while(c.read(temp,buffer_size)==buffer_size)
			std::cout<<"received packet!\t"<<temp[0]<<std::endl;

		msl::delay_ms(1);
	}

	std::cout<<"disconnected from server..."<<std::endl;

	return 0;
}
