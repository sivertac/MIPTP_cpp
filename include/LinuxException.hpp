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
		Error(std::string what);
	private:
		static std::string concat(std::string str, char* err_msg);
	};

	class BrokenPipeException : public std::exception
	{
	};

	class WouldBlockException : public std::exception
	{
	};

	class TimeoutException : public std::exception
	{
	};

	class InterruptedException : public std::exception
	{
	};
}

#endif // !LinuxException_HEADER
