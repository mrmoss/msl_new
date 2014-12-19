#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include "../socket.hpp"
#include "../time.hpp"
#include <vector>

int main()
{
	msl::socket s("127.0.0.1:8080",true);
	s.open();

	if(!s.good())
	{
		std::cout<<":("<<std::endl;
		return 0;
	}

	std::cout<<":)"<<std::endl;

	std::vector<std::pair<msl::socket,std::string>> clients;

	while(s.good())
	{
		auto client=s.accept();

		if(client.good())
		{
			std::cout<<"new client"<<std::endl;
			clients.push_back({client,""});
		}

		for(auto ii=std::begin(clients);ii!=std::end(clients);)
		{
			char temp;

			while(ii->first.available()>0&&ii->first.read(&temp,1)==1)
			{
				if(temp=='\n')
				{
					std::cout<<"client said \""<<ii->second<<"\"."<<std::endl;
					ii->first.write("\"You said \""+ii->second+"\".\n");
					ii->second="";
				}
				else
				{
					ii->second+=temp;
				}
			}

			if(!ii->first.good())
			{
				ii=clients.erase(ii);
				std::cout<<"client disconnected"<<std::endl;
			}
			else
			{
				++ii;
			}
		}
	}

	std::cout<<"server shutdown..."<<std::endl;

	return 0;
}