#include "file.hpp"

#include <fstream>

bool msl::file_to_string(const std::string& filename,std::string& data)
{
	char buffer;
	std::ifstream istr(filename.c_str(),std::ios_base::in|std::ios_base::binary);
	istr.unsetf(std::ios_base::skipws);

	if(!istr)
		return false;

	data="";

	while(istr>>buffer)
		data+=buffer;

	istr.close();
	return true;
}

bool msl::string_to_file(const std::string& data,const std::string& filename)
{
	bool saved=false;
	std::ofstream ostr(filename.c_str(),std::ios_base::out|std::ios_base::binary);
	saved=(bool)(ostr<<data);
	ostr.close();
	return saved;
}