#include <iostream>
#include "../time.hpp"
#include "../webserver.hpp"

bool client_func(const mg_connection& connection,enum mg_event event)
{
	if(event==MG_AUTH)
		return true;
	if(event!=MG_REQUEST)
		return false;

	std::cout<<connection.uri<<std::endl;
	return false;
}

int main()
{
	msl::webserver_t test(client_func,"0.0.0.0:80","web");
	test.open();

	try
	{
		test.open();
	}
	catch(std::exception& error)
	{
		std::cout<<error.what()<<std::endl;
	}

	while(true)
		msl::delay_ms(1);

	return 0;
}