//LinuxException.hpp
//Author: Sivert Andresen Cubedo
#pragma once

#ifndef LinuxException_HEADER
#define LinuxException_HEADER

#include <iostream>
#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <cassert>

#include <errno.h>
#include <string.h>

namespace LinuxException
{
	class Error : public std::runtime_error
	{
	public:
		/*
		Constructor.
		*/
		Error(std::string what);
	private:
		static std::string concat(std::string str, char* err_msg);
	};
}

#endif // !LinuxException_HEADER
