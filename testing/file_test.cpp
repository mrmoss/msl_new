#include <iostream>

#include "../file.hpp"

int main()
{
	std::cout<<msl::string_to_file("hello world!","test.txt")<<std::endl;

	std::string data;

	bool worked=msl::file_to_string("test.txt",data);

	std::cout<<worked<<std::endl;

	if(worked)
		std::cout<<data<<std::endl;

	return 0;
}