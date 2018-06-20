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

//Local
#include "CrossIPC.hpp"
#include "TimerWrapper.hpp"
#include "AddressTypes.hpp"
#include "Application.hpp"
#include "MIPTPFrame.hpp"

namespace SlidingWindow
{
	template <std::size_t N>
	class SendWindow
	{
	public:
		SendWindow(std::queue<MIPTPFrame> & out_queue, AnonymousSocket & in_sock, Port source_port, Port dest_port) :
			m_out_queue(out_queue),
			m_in_sock(in_sock),
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
				m_out_queue.push(*m_frame_container[i]);
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
					break;
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
		std::queue<MIPTPFrame> & m_out_queue;
		AnonymousSocket & m_in_sock;
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
		ReceiveWindow(std::queue<MIPTPFrame> & out_queue, std::vector<char> & msg_buffer, Port source_port, Port dest_port) :
			m_out_queue(out_queue),
			m_msg_buffer(msg_buffer),
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
		std::queue<MIPTPFrame> & m_out_queue;
		std::vector<char> & m_msg_buffer;
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
			m_out_queue.push(frame);
		}
	};
}

const std::size_t WINDOW_SIZE = 10;

class ApplicationClient
{
public:
	/*
	Constructor.
	Parameters:
		client_port		port for this client
		server_port		port to attempt handshake on
		sock
		out_queue
	*/
	ApplicationClient(Port client_port, Port server_port, AnonymousSocket & sock, std::queue<MIPTPFrame> & out_queue);

	/*
	Receive frame from sender.
	Parameters:
		frame		ref to frame.
	Return:
		void
	*/
	void receiveFrame(MIPTPFrame & frame);

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

private:
	bool m_connected = false;
	Port m_server_port;
	Port m_client_port;
	std::unique_ptr<SlidingWindow::SendWindow<WINDOW_SIZE>> m_send_window;
	std::unique_ptr<SlidingWindow::ReceiveWindow<WINDOW_SIZE>> m_receive_window;
	AnonymousSocket m_sock;
	TimerWrapper m_timer;
	std::queue<MIPTPFrame> & m_out_queue;
	std::vector<char> m_msg_buffer;
};

class ApplicationServer
{
public:
	/*
	Constructor.
	*/
	ApplicationServer(Port port, AnonymousSocket & sock, std::queue<MIPTPFrame> & out_queue);

	/*
	Receive frame from sender.
	Parameters:
		frame		ref to frame.
	Return:
		void
	*/
	void receiveFrame(MIPTPFrame & frame);

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
private:
	bool m_connected = false;
	Port m_server_port;
	Port m_client_port;
	std::unique_ptr<SlidingWindow::SendWindow<WINDOW_SIZE>> m_send_window;
	std::unique_ptr<SlidingWindow::ReceiveWindow<WINDOW_SIZE>> m_receive_window;
	AnonymousSocket m_sock;
	TimerWrapper m_timer;
	std::queue<MIPTPFrame> & m_out_queue;
	std::vector<char> m_msg_buffer;
};

#endif // !Application_HEADER
