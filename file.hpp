#ifndef MSL_C11_FILE_HPP
#define MSL_C11_FILE_HPP

#include <string>

namespace msl
{
	bool file_to_string(const std::string& filename,std::string& data);
	bool string_to_file(const std::string& data,const std::string& filename);
};

#endif