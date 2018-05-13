//RawSock.hpp
//Author: Sivert Andresen Cubedo
#pragma once
#ifndef RawSock_HEADER
#define RawSock_HEADER

#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <algorithm>

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
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <ifaddrs.h>
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
	Parameters:
		family_filter		vector containing what kind of interfaces to extract
	Return:
		Vector containing names of interfaces
	*/
	std::vector<std::string> getInterfaceNames(const std::vector<int> & family_filter);
	//std::vector<std::wstring> getInterfaceNames();		//windows uses wide char in kernel :/

}

#endif // !RawSock_HEADER
