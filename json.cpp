#include "json.hpp"

std::string msl::serialize(const msl::json& json)
{
	Json::FastWriter writer;
	return writer.write(json);
}

msl::json msl::deserialize(const std::string& str)
{
	return msl::json(str);
}