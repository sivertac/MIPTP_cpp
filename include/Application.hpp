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
#include <queue>
#include <algorithm>
#include <cassert>
#include <memory>
#include <functional>
#include <utility>

//Local
#include "CrossIPC.hpp"
#include "TimerWrapper.hpp"
#include "AddressTypes.hpp"
#include "Application.hpp"
#include "MIPTPFrame.hpp"
#include "TransportInterface.hpp"

namespace SlidingWindow
{
	template <std::size_t N>
	class SendWindow
	{
	public:
		SendWindow(std::queue<std::pair<MIPAddress, MIPTPFrame>> & out_queue, AnonymousSocket & in_sock, MIPAddress dest_mip, Port source_port, Port dest_port) :
			m_out_queue(out_queue),
			m_in_sock(in_sock),
			m_dest_mip(dest_mip),
			m_source_port(source_port),
			m_dest_port(dest_port)
		{
			assert(m_in_sock.isNonBlock());
		}

		void receiveFrame(MIPTPFrame & frame)
		{
			assert(frame.getType() == MIPTPFrame::sack);
			int seq_num = frame.getSequenceNumber();
			if (seqInsideWindow(seq_num)) {
				if (seq_num != m_seq_base) {
					moveWindow(calcMoves(seq_num));
				}
				queueFrames();
			}
		}

		void queueFrames()
		{
			for (int i = 0; i < m_used_slots; ++i) {
				m_out_queue.emplace(m_dest_mip, *m_frame_container[i]);
			}
		}

		void loadFrames()
		{
			while (m_used_slots < m_window_size) {
				MIPTPFrame & frame = *m_frame_container[m_used_slots];
				std::size_t ret;
				frame.setMsgSize(MIPTPFrame::FRAME_MAX_MSG_SIZE);
				try {
					ret = m_in_sock.read(frame.getMsg(), MIPTPFrame::FRAME_MAX_MSG_SIZE);
				}
				catch (LinuxException::WouldBlockException & e) {
					return;
				}
				frame.setType(MIPTPFrame::data);
				frame.setDest(m_dest_port);
				frame.setSource(m_source_port);
				frame.setSequenceNumber((m_seq_base + m_used_slots) % m_seq_size);
				frame.setMsgSize(ret);
				++m_used_slots;
			}
		}

	private:
		const int m_seq_size = N * 2;
		const int m_window_size = N;
		int m_used_slots = 0;
		int m_seq_base = 0;
		std::array<std::unique_ptr<MIPTPFrame>, N> m_frame_container;
		std::queue<std::pair<MIPAddress, MIPTPFrame>> & m_out_queue;
		AnonymousSocket & m_in_sock;
		const MIPAddress m_dest_mip;
		const Port m_source_port;
		const Port m_dest_port;

		void moveWindow(int moves)
		{
			if (moves > 0) {
				std::rotate(m_frame_container.begin(), m_frame_container.begin() + moves, m_frame_container.end());
				if (moves < m_used_slots) {
					m_used_slots -= moves;
				}
				else {
					m_used_slots = 0;
				}
				m_seq_base = (m_seq_base + moves) % m_seq_size;
			}
		}

		bool seqInsideWindow(int seq)
		{
			if (seq < m_seq_base) {
				if (seq + m_seq_size < m_seq_base + m_window_size) {
					return true;
				}
				else {
					return false;
				}
			}
			else {
				if (seq < m_seq_base + m_window_size) {
					return true;
				}
				else {
					return false;
				}
			}
		}

		int calcMoves(int seq) 
		{
			if (seq < m_seq_base) {
				return (seq + m_seq_size) - m_seq_base;
			}
			else {
				return seq - m_seq_base;
			}
		}
	};

	template <std::size_t N>
	class ReceiveWindow
	{
	public:
		ReceiveWindow(std::queue<std::pair<MIPAddress, MIPTPFrame>> & out_queue, std::vector<char> & msg_buffer, MIPAddress dest_mip, Port source_port, Port dest_port) :
			m_out_queue(out_queue),
			m_msg_buffer(msg_buffer),
			m_dest_mip(m_dest_mip),
			m_source_port(source_port),
			m_dest_port(dest_port)
		{
		}

		void receiveFrame(MIPTPFrame & frame)
		{
			if (frame.getSequenceNumber() == m_seq_num) {
				//if this then frame is correct frame
				m_seq_num = (m_seq_num + 1) % m_seq_size;
				m_msg_buffer.insert(m_msg_buffer.end(), frame.getMsg(), frame.getMsg() + frame.getMsgSize());
			}
			else {
				//if this then drop frame and send sack
				queueAck();
			}
		}

	private:
		const int m_seq_size = N * 2;
		int m_seq_num = 0;
		std::queue<std::pair<MIPAddress, MIPTPFrame>> & m_out_queue;
		std::vector<char> & m_msg_buffer;
		const MIPAddress m_dest_mip;
		const Port m_source_port;
		const Port m_dest_port;

		/*
		Send current ack.
		Parameters:
		Return:
			void
		*/
		void queueAck()
		{
			static MIPTPFrame frame;
			frame.setType(MIPTPFrame::sack);
			frame.setSource(m_source_port);
			frame.setDest(m_dest_port);
			frame.setSequenceNumber(m_seq_num);
			frame.setMsgSize(0);
			m_out_queue.emplace(m_dest_mip, frame);
		}
	};
}

const std::size_t WINDOW_SIZE = 10;

class ApplicationClient
{
public:
	enum ConnectStage {
		stage_application,
		stage_wait_reply,
		stage_connected,
		stage_failure
	};

	/*
	Constructor.
	Parameters:
		sock
		timer
		out_queue
		is_port_free
		get_free_port
	*/
	ApplicationClient(
		AnonymousSocket & sock, 
		TimerWrapper & timer, 
		std::queue<std::pair<MIPAddress, MIPTPFrame>> & out_queue,
		std::function<bool(Port)> is_port_free,
		std::function<Port()> get_free_port
	);

	/*
	Receive frame from sender.
	Parameters:
		frame		ref to frame.
		source		address of frame source
	Return:
		void
	*/
	void receiveFrame(MIPTPFrame & frame, MIPAddress source);

	/*
	Handle sock events.
	Parameters:
	Return:
		void
	*/
	void handleSock();

	/*
	Get server port.
	Parameters:
	Return:
		port
	*/
	Port getServerPort();

	/*
	Get client port.
	Parameters:
	Return:
		port
	*/
	Port getClientPort();

	/*
	Get ref to sock.
	Parameters:
	Return:
		ref to sock
	*/
	AnonymousSocket & getSock();

	/*
	Get ref to timer.
	Parameters:
	Return:
		ref to timer
	*/
	TimerWrapper & getTimer();

	/*
	Get stage.
	Parameters:
	Return:
		stage
	*/
	ConnectStage getStage();
private:
	ConnectStage m_stage;
	MIPAddress m_dest_mip = 0;
	Port m_server_port = 0;
	Port m_client_port = 0;
	AnonymousSocket m_sock;
	TimerWrapper m_timer;
	std::queue<std::pair<MIPAddress, MIPTPFrame>> & m_out_queue;
	std::function<bool(Port)> m_is_port_free;
	std::function<Port()> m_get_free_port;
	std::unique_ptr<SlidingWindow::SendWindow<WINDOW_SIZE>> m_send_window;
	std::unique_ptr<SlidingWindow::ReceiveWindow<WINDOW_SIZE>> m_receive_window;
	std::vector<char> m_msg_buffer;
};

class ApplicationServer
{
public:
	enum ConnectStage {
		stage_application,
		stage_listen,
		stage_connected,
		stage_failure
	};

	/*
	Constructor.
	Parameters:
		sock
		timer
		out_queue
		is_port_free
		get_free_port
	*/
	ApplicationServer(
		AnonymousSocket & sock, 
		TimerWrapper & timer, 
		std::queue<std::pair<MIPAddress, MIPTPFrame>> & out_queue,
		std::function<bool(Port)> is_port_free,
		std::function<Port()> get_free_port
	);

	/*
	Receive frame from sender.
	Parameters:
		frame		ref to frame.
		source		address of frame source
	Return:
		void
	*/
	void receiveFrame(MIPTPFrame & frame, MIPAddress source);

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
	Get server port.
	Parameters:
	Return:
		port
	*/
	Port getServerPort();

	/*
	Get client port.
	Parameters:
	Return:
		port
	*/
	Port getClientPort();

	/*
	Get ref to sock.
	Parameters:
	Return:
		ref to sock
	*/
	AnonymousSocket & getSock();

	/*
	Get ref to timer.
	Parameters:
	Return:
		ref to timer
	*/
	TimerWrapper & getTimer();

	/*
	Get stage.
	Parameters:
	Return:
		stage
	*/
	ConnectStage getStage();
private:
	ConnectStage m_stage;
	MIPAddress m_dest_mip = 0;
	Port m_server_port = 0;
	Port m_client_port = 0;
	AnonymousSocket m_sock;
	TimerWrapper m_timer;
	std::queue<std::pair<MIPAddress, MIPTPFrame>> & m_out_queue;
	std::function<bool(Port)> m_is_port_free;
	std::function<Port()> m_get_free_port;
	std::unique_ptr<SlidingWindow::SendWindow<WINDOW_SIZE>> m_send_window;
	std::unique_ptr<SlidingWindow::ReceiveWindow<WINDOW_SIZE>> m_receive_window;
	std::vector<char> m_msg_buffer;
};

#endif // !Application_HEADER
