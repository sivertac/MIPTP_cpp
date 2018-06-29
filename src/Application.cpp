//Application.cpp
//Author: Sivert Andresen Cubedo

#include "../include/Application.hpp"

ApplicationClient::ApplicationClient(AnonymousSocket & sock, TimerWrapper & timer, std::queue<std::pair<MIPAddress, MIPTPFrame>> & out_queue, std::function<bool(Port)> is_port_free, std::function<Port()> get_free_port) :
	m_stage(stage_application),
	m_sock(sock),
	m_timer(timer),
	m_out_queue(out_queue),
	m_is_port_free(is_port_free),
	m_get_free_port(get_free_port)
{
	assert(m_sock.isNonBlock());
}

void ApplicationClient::receiveFrame(MIPTPFrame & frame, MIPAddress source)
{
	if (m_stage == stage_wait_reply) {
		//if this then check for handshake reply
		if (frame.getType() == MIPTPFrame::reply) {
			//if this then 
			m_stage = stage_connected;
		}
	}
	else if (m_stage == stage_connected) {
		//else handle data or sack frame
		int type = frame.getType();
		switch (type)
		{
		case MIPTPFrame::data:
			m_receive_window->receiveFrame(frame);
			break;
		case MIPTPFrame::sack:
			m_send_window->receiveFrame(frame);
			break;
		default:
			//else drop frame
			break;
		}
	}
}

void ApplicationClient::handleSock()
{
	if (m_stage == stage_application) {
		m_sock.readGeneric(m_dest_mip);
		m_sock.readGeneric(m_server_port);
		//try to aquire a client port
		m_client_port = m_get_free_port();
		if (m_client_port == 0) {
			std::uint8_t reply = TransportInterface::reply_usedport;
			m_sock.writeGeneric(reply);
			m_sock.closeResources();
			m_stage = stage_failure;
			return;
		}
		//then send request to server
		MIPTPFrame frame;
		frame.setMsgSize(0);
		frame.setType(MIPTPFrame::request);
		frame.setDest(m_server_port);
		frame.setSource(m_client_port);
		frame.setSequenceNumber(0);
		m_out_queue.emplace(m_dest_mip, frame);
		m_stage = stage_wait_reply;

		std::cout << "transport_deamon: ApplicationClient connected\n";
		std::cout << "transport_deamon: requesting at port: " << (int)m_server_port << "\n";
		std::cout << "transport_deamon: aquired port: " << (int)m_client_port << "\n";
	}
	else if (m_stage == stage_connected) {
		//try to read
		m_send_window->loadFrames();
		//try to write data
		if (!m_msg_buffer.empty()) {
			std::size_t ret;
			try {
				ret = m_sock.write(m_msg_buffer.data(), m_msg_buffer.size());
			}
			catch (LinuxException::WouldBlockException & e) {
				return;
			}
			if (ret < m_msg_buffer.size()) {
				m_msg_buffer.erase(m_msg_buffer.begin(), m_msg_buffer.begin() + ret);
			}
			else {
				m_msg_buffer.clear();
			}
		}
	}
}

Port ApplicationClient::getServerPort()
{
	return m_server_port;
}

Port ApplicationClient::getClientPort()
{
	return m_client_port;
}

AnonymousSocket & ApplicationClient::getSock()
{
	return m_sock;
}

TimerWrapper & ApplicationClient::getTimer()
{
	return m_timer;
}

ApplicationClient::ConnectStage ApplicationClient::getStage()
{
	return m_stage;
}

/*
ApplicationServer:
*/
ApplicationServer::ApplicationServer(AnonymousSocket & sock, TimerWrapper & timer, std::queue<std::pair<MIPAddress, MIPTPFrame>>& out_queue, std::function<bool(Port)> is_port_free, std::function<Port()> get_free_port) :	
	m_stage(stage_application),
	m_sock(sock),
	m_timer(timer),
	m_out_queue(out_queue),
	m_is_port_free(is_port_free),
	m_get_free_port(get_free_port)
{
	assert(m_sock.isNonBlock());
}

void ApplicationServer::receiveFrame(MIPTPFrame & frame, MIPAddress source)
{
	if (m_stage == stage_listen) {
		//if this then handle handshake request
		if (frame.getType() == MIPTPFrame::request) {
			m_client_port = frame.getSource();
			m_dest_mip = source;
			//queue reply
			frame.setType(MIPTPFrame::reply);
			frame.setSource(m_server_port);
			frame.setDest(m_client_port);
			frame.setMsgSize(0);
			m_out_queue.emplace(m_dest_mip, frame);
			//init windows
			m_send_window = std::unique_ptr<SlidingWindow::SendWindow<WINDOW_SIZE>>(
				new SlidingWindow::SendWindow<WINDOW_SIZE>(m_out_queue, m_sock, m_dest_mip, m_server_port, m_client_port));
			m_receive_window = std::unique_ptr<SlidingWindow::ReceiveWindow<WINDOW_SIZE>>(
				new SlidingWindow::ReceiveWindow<WINDOW_SIZE>(m_out_queue, m_msg_buffer, m_dest_mip, m_server_port, m_client_port));
			m_stage = stage_connected;
		}
	}
	else if (m_stage == stage_connected) {
		//else handle data or sack frame
		int type = frame.getType();
		switch (type)
		{
		case MIPTPFrame::data:
			m_receive_window->receiveFrame(frame);
			break;
		case MIPTPFrame::sack:
			m_send_window->receiveFrame(frame);
			break;
		default:
			//else drop frame
			break;
		}
	}
}

void ApplicationServer::handleSock()
{
	if (m_stage == stage_application) {
		Port port;
		m_sock.readGeneric(port);
		if (m_is_port_free(port)) {
			//if this then set server port and continue
			m_server_port = port;
			std::uint8_t reply = TransportInterface::reply_success;
			m_sock.writeGeneric(reply);
			m_stage = stage_listen;

			std::cout << "transport_deamon: ApplicationServer connected\n";
			std::cout << "transport_deamon: listening at port: " << (int)m_server_port << "\n";
		}
		else {
			//if this decline request and set stage to failure
			std::uint8_t reply = TransportInterface::reply_usedport;
			m_sock.writeGeneric(reply);
			m_sock.closeResources();
			m_stage = stage_failure;
		}
	}
	else if (m_stage == stage_connected) {
		//try to read
		m_send_window->loadFrames();
		//try to write data
		if (!m_msg_buffer.empty()) {
			std::size_t ret;
			try {
				ret = m_sock.write(m_msg_buffer.data(), m_msg_buffer.size());
			}
			catch (LinuxException::WouldBlockException & e) {
				return;
			}
			if (ret < m_msg_buffer.size()) {
				m_msg_buffer.erase(m_msg_buffer.begin(), m_msg_buffer.begin() + ret);
			}
			else {
				m_msg_buffer.clear();
			}
		}
	}
}

void ApplicationServer::handleTimer()
{
	if (m_stage == stage_connected) {
		m_send_window->queueFrames();
	}
}

Port ApplicationServer::getServerPort()
{
	return m_server_port;
}

Port ApplicationServer::getClientPort()
{
	return m_client_port;
}

AnonymousSocket & ApplicationServer::getSock()
{
	return m_sock;
}

TimerWrapper & ApplicationServer::getTimer()
{
	return m_timer;
}

ApplicationServer::ConnectStage ApplicationServer::getStage()
{
	return m_stage;
}

