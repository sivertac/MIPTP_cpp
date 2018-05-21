//EthernetFrame.cpp
//Author: Sivert Andresen Cubedo

#include "../include/EthernetFrame.hpp"

EthernetFrame::EthernetFrame() :
	m_msg_size(0),
	m_data(FRAME_HEADER_SIZE)
{
}

EthernetFrame::EthernetFrame(char* buf, std::size_t size)
{
	assert(size >= FRAME_HEADER_SIZE);
	if (m_data.size() != size) {
		m_data.resize(size);
	}
	std::memcpy(m_data.data(), buf, size);
	m_msg_size = size - FRAME_HEADER_SIZE;
}

void EthernetFrame::setDest(MACAddress & dest)
{
	assert(m_data.size() >= FRAME_HEADER_SIZE);
	std::copy(dest.begin(), dest.end(), m_data.begin());
}

void EthernetFrame::setSource(MACAddress & source)
{
	assert(m_data.size() >= FRAME_HEADER_SIZE);
	std::copy(source.begin(), source.end(), m_data.begin() + 6);
}

void EthernetFrame::setProtocol(int protocol)
{
	assert(m_data.size() >= FRAME_HEADER_SIZE);
	std::memcpy(m_data.data() + 12, (char*)&protocol, 2);
}

void EthernetFrame::setSize(std::size_t size)
{
	assert(size >= FRAME_HEADER_SIZE);
	setMsgSize(size - FRAME_HEADER_SIZE);
}

void EthernetFrame::setMsgSize(std::size_t size)
{
	m_msg_size = size;
	if (m_data.size() - FRAME_HEADER_SIZE != m_msg_size) {
		m_data.resize(m_msg_size + FRAME_HEADER_SIZE);
	}
}

void EthernetFrame::setMsg(const char* buf, std::size_t size)
{
	if (size != getMsgSize()) {
		setMsgSize(size);
	}
	if (size > 0) {
		std::memcpy(m_data.data() + 14, buf, size);
	}
}

MACAddress EthernetFrame::getDest()
{
	assert(m_data.size() >= FRAME_HEADER_SIZE);
	MACAddress dest;
	std::copy(m_data.begin(), m_data.begin() + 6, dest.begin());
	return dest;
}

MACAddress EthernetFrame::getSource()
{
	assert(m_data.size() >= FRAME_HEADER_SIZE);
	MACAddress source;
	std::copy(m_data.begin() + 6, m_data.begin() + 12, source.begin());
	return source;
}

int EthernetFrame::getProtocol()
{
	assert(m_data.size() >= FRAME_HEADER_SIZE);
	int protocol;
	std::memcpy((char*)&protocol, m_data.data() + 12, 2);
	return protocol;
}

std::size_t EthernetFrame::getMsgSize()
{
	return m_msg_size;
}

char* EthernetFrame::getMsg()
{
	return m_data.data() + FRAME_HEADER_SIZE;
}



std::size_t EthernetFrame::getSize()
{
	return m_msg_size + FRAME_HEADER_SIZE;
}

char* EthernetFrame::getData()
{
	return m_data.data();
}

std::vector<char>& EthernetFrame::getVector()
{
	return m_data;
}

std::string EthernetFrame::toString()
{
	std::stringstream ss;
	ss << "Dest: " << toStringMACAddress(getDest()) << "\n"
		<< "Source: " << toStringMACAddress(getSource()) << "\n"
		<< "Protocol: " << getProtocol() << "\n"
		<< "Msg size: " << getMsgSize() << "\n"
		<< "Frame size: " << getSize();
	return ss.str();
}
