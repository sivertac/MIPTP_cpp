//ClientHandler.hpp
//Author: Sivert Andresen Cubedo

#pragma once
#ifndef ClientHandler_HEADER
#define CLientHandler_HEADER

//C++
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <functional>

#include "CrossIPC.hpp"
#include "TimerWrapper.hpp"

class ClientHandler
{
public:
	enum ClientType
	{
		type_server,
		type_client
	};

	enum ConnectStage
	{
		stage_application,	//client server
		stage_listen,		//server
		stage_wait_reply,	//client
		stage_connected,	//client server
		stage_failure		//client server
	};

	/*
	Constructor.
	*/
	ClientHandler(
		AnonymousSocket & sock, 
		TimerWrapper & timer, 
		std::queue<std::pair<MIPAddress, MIPTPFrame>> & out_queue,
		std::function<bool(Port)> is_port_free,
		std::function<Port()> get_free_port
	);



	/*
	Get ref to timer.
	Parameters:
	Return:
		ref to m_timer
	*/
	TimerWrapper & getTimer();

	/*
	Get ref to sock.
	Parameters:
	Return:
		ref to m_sock
	*/
	AnonymousSocket & getSock();

	/*
	Get stage of object.
	Parameters:
	Return:
		stage
	*/
	ConnectStage getStage();
private:
	ClientType type;
	ConnectStage stage;
	AnonymousSocket m_sock;
	TimerWrapper m_timer;
};

#endif // !ClientHandler_HEADER
