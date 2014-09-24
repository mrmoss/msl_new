#include "string.hpp"

#include <algorithm>
#include <ctype.h>

std::string msl::to_lower(std::string str)
{
	std::transform(str.begin(),str.end(),str.begin(),tolower);
	return str;
}

std::string msl::to_upper(std::string str)
{
	std::transform(str.begin(),str.end(),str.begin(),toupper);
	return str;
}

std::string msl::replace_all(std::string str,const std::string& find,const std::string& replace)
{
	size_t found=str.find(find,0);

	while(found!=std::string::npos)
	{
		str.replace(found,find.size(),replace);
		found=str.find(find,found-find.size()+replace.size());
	}

	return str;
}

bool msl::ends_with(const std::string& str,const std::string& suffix)
{
	return str.rfind(suffix)==(str.size()-suffix.size());
}

bool msl::starts_with(const std::string& str,const std::string& prefix)
{
	return str.find(prefix)==0;
}