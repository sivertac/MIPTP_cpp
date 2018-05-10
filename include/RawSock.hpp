//RawSock.hpp
//Author: Sivert Andresen Cubedo
#pragma once
#ifndef RawSock_HEADER
#define RawSock_HEADER

#include <iostream>
#include <string>
#include <vector>
#include <exception>

//platform spesific
#ifdef WINDOWS
//windows stuff
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms740548(v=vs.85).aspx
#include <Windows.h>
#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#elif LINUX
//unix stuff
#else
//error
#error CrossIPC.hpp: Not defined target OS
#endif

namespace RawSock
{
	class RawSock
	{

	};

	/*
	Get network interface names on this host.
	*/
	std::vector<std::string> getInterfaceNames();

}

#endif // !RawSock_HEADER
