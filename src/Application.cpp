//Application.cpp
//Author: Sivert Andresen Cubedo

#include "../include/Application.hpp"

ApplicationClient::ApplicationClient(AnonymousSocket & sock, TimerWrapper & timer, std::queue<std::pair<MIPAddress, MIPTPFrame>> & out_queue, std::function<bool(Port)> is_port_free, std::function<Port()> get_free_port) :
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
	if (!m_connected) {
		//if this then handshake reply
		if (frame.getType() == MIPTPFrame::reply) {
			m_connected = true;
		}
	}
	else {
		//if this then handle data or sack
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
	if (m_connected) {
		//read block
		m_send_window->loadFrames();
		//send until block
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




ApplicationServer::ApplicationServer(AnonymousSocket & sock, TimerWrapper & timer, std::queue<std::pair<MIPAddress, MIPTPFrame>>& out_queue, std::function<bool(Port)> is_port_free, std::function<Port()> get_free_port) :	
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
	if (!m_connected) {
		//if this then handle handshake request
		if (frame.getType() == MIPTPFrame::request) {
			m_client_port = frame.getSource();
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
			m_connected = true;
		}
	}
	else {
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
	if (m_connected) {
		//read block
		m_send_window->loadFrames();
		//send until block
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
	if (m_connected) {
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

