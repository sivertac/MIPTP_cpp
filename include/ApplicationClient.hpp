//ApplicationClient.hpp
//Author: Sivert Andresen Cubedo

#pragma once
#ifndef ApplicationClient_HEADER
#define ApplicationClient_HEADER

//C++
#include <iostream>
#include <sstream>
#include <string>

//Local
#include "../include/CrossIPC.hpp"

using Port = std::uint16_t;

class ApplicationClient
{
public:
	/*
	Constructor.
	*/
	ApplicationClient();

	/*
	Constructor.
	Parameters:
		port
		sock
	*/
	ApplicationClient(Port port, AnonymousSocket && sock);
private:
	AnonymousSocket m_sock;
};

#endif // !ApplicationClient_HEADER
