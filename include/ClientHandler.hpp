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
#include <array>
#include <memory>

#include "CrossIPC.hpp"
#include "TimerWrapper.hpp"
#include "MIPTPFrame.hpp"
#include "AddressTypes.hpp"
#include "TransportInterface.hpp"

class ClientHandler
{
public:
	enum ClientType
	{
		type_null,
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
		int timeout,
		std::queue<std::pair<MIPAddress, MIPTPFrame>> & out_queue,
		std::function<bool(Port)> is_port_free,
		std::function<Port()> get_free_port
	);

	/*
	Destructor.
	*/
	~ClientHandler();

	/*
	Handle sock events.
	Parameters:
	Return:
		void
	*/
	void handleSock();

	/*
	Handle timer events.
	Parameters:
	Return:
		void
	*/
	void handleTimer();

	/*
	Receive frame.
	Parameters:
		source	source address of frame
		frame	ref to frame
	Return:
		void
	*/
	void receiveFrame(MIPAddress source, MIPTPFrame & in_frame);

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
	Get type of client.
	Parameters:
	Return:
		ClientType
	*/
	ClientType getType();

	/*
	Get stage of object.
	Parameters:
	Return:
		stage
	*/
	ConnectStage getStage();

	/*
	Get source port.
	Parameters:
	Return:
		port
	*/
	Port getSourcePort();

private:
	ClientType m_type;
	ConnectStage m_stage;

	AnonymousSocket m_sock;
	TimerWrapper m_timer;
	
	int m_timeout;	//in ms

	std::queue<std::pair<MIPAddress, MIPTPFrame>> & m_out_queue;

	std::function<bool(Port)> m_is_port_free;
	std::function<Port()> m_get_free_port;

	MIPAddress m_dest_address = 0;
	Port m_dest_port = 0;
	Port m_source_port = 0;

	//sliding window
	static const int m_window_size = 10;
	static const int m_sequence_size = m_window_size * 4;
	int m_used_slots = 0;
	int m_sequence_base = 0;
	int m_current_ack = 0;
	std::array<std::unique_ptr<MIPTPFrame>, m_window_size> m_frame_window;
	std::vector<char> m_msg_buffer;

	std::size_t m_total_data_loaded = 0;
	std::size_t m_total_data_received = 0;

	bool m_connected_reply = true;

	/*
	Client: Load frames in to window.
	Parameters:
	Return:
		void
	*/
	void loadFrames();

	/*
	Client: Queue frames in window.
	Parameters:
	Return:
		void
	*/
	void queueWindow();

	/*
	Client: Move window n spaces.
	Parameters:
		moves	number of moves
	Return:
		void
	*/
	void moveWindow(int moves);

	/*
	Client: Check if sequence number is inside window.
	Parameters:
		seq		number to check
	Return:
		bool
	*/
	bool seqInsideWindow(int seq);

	/*
	Client: Calculate distance from m_sequence_base to seq.
	Parameters:
		seq		
	Return:
		moves
	*/
	int calcMoves(int seq);

	/*
	Server: Queue ack.
	Parameters:
	Return:
		void
	*/
	void queueAck();

	/*
	Server: Send data to sock.
	Parameters:
	Return:
		void
	*/
	void sendToSock();
};

#endif // !ClientHandler_HEADER
