//Application.cpp
//Author: Sivert Andresen Cubedo

#include "../include/Application.hpp"

ApplicationClient::ApplicationClient(Port client_port, Port server_port, AnonymousSocket & sock, std::queue<MIPTPFrame>& out_queue) :
	m_server_port(server_port),
	m_client_port(client_port),
	m_sock(sock),
	m_out_queue(out_queue)
{
	assert(m_sock.isNonBlock());
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

ApplicationServer::ApplicationServer(Port port, AnonymousSocket & sock, std::queue<MIPTPFrame>& out_queue) :
	m_server_port(port),
	m_sock(sock),
	m_out_queue(out_queue)
{
	assert(m_sock.isNonBlock());
}

void ApplicationServer::receiveFrame(MIPTPFrame & frame)
{
	if (!m_connected) {
		//if this then handle handshake reply
		if (frame.getType() == MIPTPFrame::request) {
			m_client_port = frame.getSource();
			//queue reply
			frame.setType(MIPTPFrame::reply);
			frame.setSource(m_server_port);
			frame.setDest(m_client_port);
			frame.setMsgSize(0);
			m_out_queue.push(frame);
			//init windows
			m_send_window = std::unique_ptr<SlidingWindow::SendWindow<WINDOW_SIZE>>(
				new SlidingWindow::SendWindow<WINDOW_SIZE>(m_out_queue, m_sock, m_server_port, m_client_port));
			m_receive_window = std::unique_ptr<SlidingWindow::ReceiveWindow<WINDOW_SIZE>>(
				new SlidingWindow::ReceiveWindow<WINDOW_SIZE>(m_out_queue, m_msg_buffer, m_server_port, m_client_port));
			m_connected = true;
		}
	}
	else {
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

