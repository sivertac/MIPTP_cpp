//LinuxException.cpp
//Author: Sivert Andresen Cubedo

#include "../include/LinuxException.hpp"

namespace LinuxException
{
	Error::Error(std::string what) :
		std::runtime_error(concat(what, strerror(errno)))
	{
	}

	std::string Error::concat(std::string str, char * err_msg)
	{
		std::stringstream ss;
		ss << str << ":  " << err_msg;
		return ss.str();
	}
}