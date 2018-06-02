//Application.hpp
//Author: Sivert Andresen Cubedo

#pragma once
#ifndef Application_HEADER
#define Application_HEADER

//C++
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <thread>

//Local
#include "../include/CrossIPC.hpp"
#include "../include/AddressTypes.hpp"

class Application
{
public:
	/*
	Constructor.
	*/
	Application(Port port, AnonymousSocket & sock);

	/*
	Receive from network.
	*/
	virtual void receivePacket(std::vector<char> & buf) = 0;

	/*
	Send to network.
	*/
	virtual void sendPacket(std::vector<char> & buf) = 0;

	/*
	Get port.
	Parameters:
	Return:
		port
	*/
	virtual Port getPort();

	/*
	Get ref to sock.
	Parameters:
	Return:
		ref to m_sock
	*/
	virtual AnonymousSocket & getSock();
private:
	static const std::size_t WINDOW_SIZE = 10;
	static const std::size_t WINDOW_CAPACITY = WINDOW_SIZE * 2;
	static const std::size_t PACKET_MAX_SIZE = 1450;
	Port m_port;
	AnonymousSocket m_sock;

	//std::array<std::array<char, PACKET_MAX_SIZE>, WINDOW_CAPACITY> m_data_container;
	//std::array<
};

class ApplicationClient : public Application
{
public:
	/*
	Constructor.
	Parameters:
		port
		sock
	*/
	ApplicationClient(Port port, AnonymousSocket & sock);

private:
	Port m_port;
	AnonymousSocket m_sock;
};

class ApplicationServer : public Application
{
public:
	/*
	Constructor.
	*/
	ApplicationServer(Port port, AnonymousSocket & sock);

private:
	Port m_port;
	AnonymousSocket m_sock;
};

#endif // !Application_HEADER
