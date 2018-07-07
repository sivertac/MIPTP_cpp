//ClientHandler.cpp
//Author: Sivert Andresen Cubedo

#include "../include/ClientHandler.hpp"

ClientHandler::ClientHandler(AnonymousSocket & sock, TimerWrapper & timer, int timeout, std::queue<std::pair<MIPAddress, MIPTPFrame>>& out_queue, std::function<bool(Port)> is_port_free, std::function<Port()> get_free_port) :
	m_type(type_null),
	m_stage(stage_application),
	m_sock(sock),
	m_timer(timer),
	m_timeout(timeout),
	m_out_queue(out_queue),
	m_is_port_free(is_port_free),
	m_get_free_port(get_free_port),
	m_dest_address(0),
	m_dest_port(0),
	m_source_port(0),
	m_used_slots(0),
	m_sequence_base(0),
	m_current_ack(0),
	m_total_data_loaded(0),
	m_total_data_received(0)
{
	assert(m_sock.isNonBlock());
}

ClientHandler::~ClientHandler()
{
	if (!m_sock.isClosed()) {
		m_sock.closeResources();
	}
	if (!m_timer.isClosed()) {
		m_timer.closeResources();
	}
}

void ClientHandler::handleSock()
{
	if (m_stage == stage_application) {
		try {
			if (m_type == type_null) {
				std::uint8_t request;
				m_sock.readGeneric(request);
				if (request == TransportInterface::request_listen) {
					m_type = type_server;
				}
				else if (request == TransportInterface::request_connect) {
					m_type = type_client;
				}
			}
			if (m_type == type_server) {
				//handle listen request
				if (m_source_port == 0) {
					Port port;
					m_sock.readGeneric(port);
					if (m_is_port_free(port)) {
						m_source_port = port;
						m_stage = stage_listen;
					}
					else {
						m_stage = stage_failure;
						std::uint8_t reply = TransportInterface::reply_usedport;
						m_sock.writeGeneric(reply);
						return;
					}
				}
			}
			else if (m_type == type_client) {
				//handle connect request
				if (m_dest_address == 0) {
					m_sock.readGeneric(m_dest_address);
				}
				if (m_dest_port == 0) {
					m_sock.readGeneric(m_dest_port);
				}
				if (m_source_port == 0) {
					m_source_port = m_get_free_port();
				}
				if (m_source_port > 0) {
					//init request frame
					MIPTPFrame frame;
					frame.setType(MIPTPFrame::request);
					frame.setDest(m_dest_port);
					frame.setSource(m_source_port);
					frame.setSequenceNumber(0);
					frame.setMsgSize(0);
					//send
					m_out_queue.emplace(m_dest_address, frame);
					//set timeout
					m_timer.setExpirationFromNow(m_timeout);
					//set stage
					m_stage = stage_wait_reply;
				}
				else {
					m_stage = stage_failure;
					std::uint8_t reply = TransportInterface::reply_usedport;
					m_sock.writeGeneric(reply);
					return;
				}
			}
			else {
				std::cerr << "transport_deamon: Invalid client request\n";
				assert(false);
			}
		}
		catch (LinuxException::WouldBlockException & e) {
			return;
		}
		catch (LinuxException::BrokenPipeException & e) {
			m_stage = stage_failure;
			return;
		}
	}
	else if (m_stage == stage_connected) {
		if (m_type == type_server) {
			//try to send to sock
			sendToSock();
		}
		else if (m_type == type_client) {
			//try to load
			loadFrames();
		}
	}
}

void ClientHandler::handleTimer()
{
	if (m_stage == stage_wait_reply) {
		//if this then we have not gotten a reply within the timeout
		//reply timeout
		std::uint8_t reply = TransportInterface::reply_timeout;
		m_sock.writeGeneric(reply);
		m_stage = stage_failure;
	}
	else if (m_stage == stage_connected) {
		//try to handle sock
		handleSock();
		if (m_type == type_server) {
			//send ack
			queueAck();
			//try to send to sock
			sendToSock();
		}
		else if (m_type == type_client) {
			//send window
			queueWindow();
		}
		else {
			assert(false);
		}
		//check if connection is maintained
		if (!m_connected_reply) {
			//if this then no reply
			m_stage = stage_failure;
		}
		else {
			static MIPTPFrame request_frame;
			request_frame.setType(MIPTPFrame::request);
			request_frame.setDest(m_dest_port);
			request_frame.setSource(m_source_port);
			request_frame.setSequenceNumber(0);
			request_frame.setMsgSize(0);
			m_out_queue.emplace(m_dest_address, request_frame);
			//reset var
			m_connected_reply = false;
		}
	}
	m_timer.setExpirationFromNow(m_timeout);
}

void ClientHandler::receiveFrame(MIPAddress source, MIPTPFrame & in_frame)
{
	if (m_stage == stage_listen) {
		if (in_frame.getType() == MIPTPFrame::request) {
			//store addresses
			m_dest_address = source;
			m_dest_port = in_frame.getSource();
			//init reply
			MIPTPFrame out_frame;
			out_frame.setType(MIPTPFrame::reply);
			out_frame.setDest(m_dest_port);
			out_frame.setSource(m_source_port);
			out_frame.setSequenceNumber(0);
			out_frame.setMsgSize(0);
			//send
			m_out_queue.emplace(m_dest_address, out_frame);
			try {
				//reply sock
				std::uint8_t reply = TransportInterface::reply_success;
				m_sock.writeGeneric(reply);
			}
			catch (LinuxException::WouldBlockException & e) {
				m_stage = stage_failure;
				return;
			}
			catch (LinuxException::BrokenPipeException & e) {
				m_stage = stage_failure;
				return;
			}
			//arm timer
			m_timer.setExpirationFromNow(m_timeout);
			//set stage
			m_stage = stage_connected;

			std::cout << "transport_deamon: server connected\n";
			std::cout << "transport_deamon: source_port: " << (int)m_source_port
				<< " dest_mip: " << (int)m_dest_address
				<< " dest_port: " << (int)m_dest_port << "\n";
		}
	}
	else if (m_stage == stage_wait_reply) {
		if (in_frame.getType() == MIPTPFrame::reply) {
			try {
				//reply sock
				std::uint8_t reply = TransportInterface::reply_success;
				m_sock.writeGeneric(reply);
			}
			catch (LinuxException::WouldBlockException & e) {
				m_stage = stage_failure;
				return;
			}
			catch (LinuxException::BrokenPipeException & e) {
				m_stage = stage_failure;
				return;
			}
			//init window
			std::for_each(m_frame_window.begin(), m_frame_window.end(), [](std::unique_ptr<MIPTPFrame> & ptr) {ptr = std::unique_ptr<MIPTPFrame>(new MIPTPFrame()); });

			//set stage
			m_stage = stage_connected;

			std::cout << "transport_deamon: client connected\n";
			std::cout << "transport_deamon: source_port: " << (int)m_source_port
				<< " dest_mip: " << (int)m_dest_address
				<< " dest_port: " << (int)m_dest_port << "\n";
		}
	}
	else if (m_stage == stage_connected) {
		//check if frame is from correct source
		if (source == m_dest_address && in_frame.getSource() == m_dest_port) {
			int frame_type = in_frame.getType();
			if (m_type == type_server) {
				if (frame_type == MIPTPFrame::data) {
					if (in_frame.getSequenceNumber() == m_current_ack) {
						m_current_ack = (m_current_ack + 1) % m_sequence_size;
						m_msg_buffer.insert(m_msg_buffer.end(), in_frame.getMsg(), in_frame.getMsg() + in_frame.getMsgSize());
						m_total_data_received += in_frame.getMsgSize();
					}
					else {
						queueAck();
					}
				}
			}
			else if (m_type == type_client) {
				if (frame_type == MIPTPFrame::sack) {
					int seq_num = in_frame.getSequenceNumber();
					if (seqInsideWindow(seq_num) || seq_num == (m_sequence_base + m_window_size) % m_sequence_size) {
						if (seq_num != m_sequence_base) {
							moveWindow(calcMoves(seq_num));
						}
						queueWindow();
					}
				}
			}
			//check if request_frame
			if (frame_type == MIPTPFrame::request) {
				//if request then reply
				static MIPTPFrame reply_frame;
				reply_frame.setType(MIPTPFrame::reply);
				reply_frame.setDest(m_dest_port);
				reply_frame.setSource(m_source_port);
				reply_frame.setSequenceNumber(0);
				reply_frame.setMsgSize(0);
				m_out_queue.emplace(m_dest_address, reply_frame);
				m_connected_reply = true;	//if we receive connect then we're connected too (redundancy in case we lose a request)
			}
			else if (frame_type == MIPTPFrame::reply) {
				//if reply then we're still connected
				m_connected_reply = true;
			}
		}
		//else drop frame
	}
}

TimerWrapper & ClientHandler::getTimer()
{
	return m_timer;
}

AnonymousSocket & ClientHandler::getSock()
{
	return m_sock;
}

ClientHandler::ClientType ClientHandler::getType()
{
	return m_type;
}

ClientHandler::ConnectStage ClientHandler::getStage()
{
	return m_stage;
}

Port ClientHandler::getSourcePort()
{
	return m_source_port;
}

void ClientHandler::loadFrames()
{
	if (!m_sock.isClosed()) {
		try {
			while (m_used_slots < m_window_size) {
				MIPTPFrame & frame = *(m_frame_window[m_used_slots]);
				frame.setMsgSize(MIPTPFrame::FRAME_MAX_MSG_SIZE);
				std::size_t ret = m_sock.read(frame.getMsg(), MIPTPFrame::FRAME_MAX_MSG_SIZE);
				frame.setType(MIPTPFrame::data);
				frame.setDest(m_dest_port);
				frame.setSource(m_source_port);
				frame.setSequenceNumber((m_sequence_base + m_used_slots) % m_sequence_size);
				frame.setMsgSize(ret);
				++m_used_slots;
				m_total_data_loaded += frame.getMsgSize();
			}
		}
		catch (LinuxException::WouldBlockException & e) {
			//do nothing
		}
		catch (LinuxException::BrokenPipeException & e) {
			if (m_used_slots == 0) {
				m_stage = stage_failure;
			}
		}
	}
}

void ClientHandler::queueWindow()
{
	for (int i = 0; i < m_used_slots; ++i) {
		m_out_queue.emplace(m_dest_address, *(m_frame_window[i]));
	}
}

void ClientHandler::moveWindow(int moves)
{
	if (moves > 0) {
		std::rotate(m_frame_window.begin(), m_frame_window.begin() + moves, m_frame_window.end());
		if (moves < m_used_slots) {
			m_used_slots -= moves;
		}
		else {
			m_used_slots = 0;
		}
		m_sequence_base = (m_sequence_base + moves) % m_sequence_size;
	}
}

bool ClientHandler::seqInsideWindow(int seq)
{
	if (seq < m_sequence_base) {
		if (seq + m_sequence_size < m_sequence_base + m_window_size) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		if (seq < m_sequence_base + m_window_size) {
			return true;
		}
		else {
			return false;
		}
	}
}

int ClientHandler::calcMoves(int seq)
{
	if (seq < m_sequence_base) {
		return (seq + m_sequence_size) - m_sequence_base;
	}
	else {
		return seq - m_sequence_base;
	}
}

void ClientHandler::queueAck()
{
	static MIPTPFrame frame;
	frame.setType(MIPTPFrame::sack);
	frame.setDest(m_dest_port);
	frame.setSource(m_source_port);
	frame.setSequenceNumber(m_current_ack);
	frame.setMsgSize(0);
	m_out_queue.emplace(m_dest_address, frame);
}

void ClientHandler::sendToSock()
{
	try {
		std::size_t ret;
		if (!m_msg_buffer.empty()) {
			ret = m_sock.write(m_msg_buffer.data(), m_msg_buffer.size());
		}
		if (ret < m_msg_buffer.size()) {
			m_msg_buffer.erase(m_msg_buffer.begin(), m_msg_buffer.begin() + ret);
		}
		else {
			m_msg_buffer.clear();
		}
	}
	catch (LinuxException::WouldBlockException & e) {
		//do nothing
	}
	catch (LinuxException::BrokenPipeException & e) {
		m_stage = stage_failure;
	}
}
	