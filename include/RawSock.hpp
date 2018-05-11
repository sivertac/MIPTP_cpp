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
#include <Winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>		//NEED TO LINK Iphlpapi.lib
#include <stdio.h>
#include <Windows.h>
#elif LINUX
//unix stuff
#else
//error
#error RawSock.hpp: Not defined target OS
#endif

namespace RawSock
{
	class RawSock
	{

	};

	/*
	Get network interface names on this host.
	*/
#ifdef WINDOWS
	std::vector<std::wstring> getInterfaceNames();
#elif LINUX
	std::vector<std::string> getInterfaceNames();
#endif

}

#endif // !RawSock_HEADER
