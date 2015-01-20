#include "webserver.hpp"

#include <chrono>
#include <thread>

msl::webserver_t::webserver_t(client_func_t client_func,const std::string& address,const std::string& webroot):
	server_m(nullptr),client_func_m(client_func),address_m(address),webroot_m(webroot)
{}

msl::webserver_t::~webserver_t()
{
	close();
}

bool msl::webserver_t::good() const
{
	return server_m!=nullptr;
}

void msl::webserver_t::open()
{
	close();
	auto server=mg_create_server(this,msl::webserver_t::client_func_handler);
	mg_set_option(server,"listening_port",address_m.c_str());
	mg_set_option(server,"document_root",webroot_m.c_str());

	if(server!=nullptr&&mg_poll_server(server,10))
		server_m=server;

	if(good())
	{
		std::thread server_thread(&msl::webserver_t::server_thread_func_m,this);
		server_thread.detach();
	}
}

void msl::webserver_t::close()
{
	if(server_m!=nullptr)
	{
		mg_destroy_server(&server_m);
		server_m=nullptr;
	}
}

std::string msl::webserver_t::address() const
{
	return address_m;
}

std::string msl::webserver_t::webroot() const
{
	return webroot_m;
}

int msl::webserver_t::client_func_handler(mg_connection* connection,enum mg_event event)
{
	if(connection!=nullptr)
		return ((msl::webserver_t*)(connection->server_param))->client_func_m(*connection,event);

	return false;
}

void msl::webserver_t::server_thread_func_m()
{
	while(mg_poll_server(server_m,10))
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

	close();
}