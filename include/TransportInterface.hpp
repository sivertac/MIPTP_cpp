//TransportInterface.hpp
//Author: Sivert Andresen Cubedo
#pragma once
#ifndef TransportInterface_HEADER
#define TransportInterface_HEADER

#include <iostream>
#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>

#include "AddressTypes.hpp"
#include "CrossIPC.hpp"

namespace TransportInterface {
	enum ApplicationRequest
	{
		request_listen = 1,
		request_connect = 2
	};

	enum ApplicationReply
	{
		reply_success = 1,
		reply_timeout = 2,
		reply_usedport = 3
	};

	/*
	Port in use exception.
	*/
	class PortInUseException : public std::exception
	{
	};

	/*
	Timeout exception.
	*/
	class TimeoutException : public std::exception
	{
	};

	/*
	Request listen on port (server).
	Parameters:
		path	path of namedsocket that transport_deamon is listening on
		port	port to request
	Return:
		AnonymousSocket
	*/
	AnonymousSocket requestListen(std::string path, Port port);

	/*
	Request connect on MIPAddress and port (client).
	Parameters:
		path	path of namedsocket that transport_deamon is listening on
		address	address of server
		port	port of server
	Return:
		AnonymousSocket
	*/
	AnonymousSocket requestConnect(std::string path, MIPAddress address, Port port);
}

#endif // !TransportInterface_HEADER
