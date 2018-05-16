//EthernetFrame.cpp
//Author: Sivert Andresen Cubedo

#include "../include/EthernetFrame.hpp"

EthernetFrame::EthernetFrame() :
	m_msg_size(0),
	m_data(14)
{
}

EthernetFrame::EthernetFrame(char* buf, std::size_t size)
{
	assert(size >= 14);
	if (m_data.size() != size) {
		m_data.resize(size);
	}
	std::memcpy(m_data.data(), buf, size);
	m_msg_size = size - 14;
}

void EthernetFrame::setDest(MACAddress & dest)
{
	assert(m_data.size() >= 14);
	std::copy(dest.begin(), dest.end(), m_data.begin());
}

void EthernetFrame::setSource(MACAddress & source)
{
	assert(m_data.size() >= 14);
	std::copy(source.begin(), source.end(), m_data.begin() + 6);
}

void EthernetFrame::setProtocol(int protocol)
{
	assert(m_data.size() >= 14);
	std::memcpy(m_data.data() + 12, (char*)&protocol, 2);
}

void EthernetFrame::setMsgSize(std::size_t size)
{
	m_msg_size = size;
	if (m_data.size() - 14 != m_msg_size) {
		m_data.resize(m_msg_size + 14);
	}
}

MACAddress EthernetFrame::getDest()
{
	assert(m_data.size() >= 14);
	MACAddress dest;
	std::copy(m_data.begin(), m_data.begin() + 6, dest.begin());
	return dest;
}

MACAddress EthernetFrame::getSource()
{
	assert(m_data.size() >= 14);
	MACAddress source;
	std::copy(m_data.begin() + 6, m_data.begin() + 12, source.begin());
	return source;
}

int EthernetFrame::getProtocol()
{
	assert(m_data.size() >= 14);
	int protocol = 0;
	std::memcpy((char*)&protocol, m_data.data() + 12, 2);
	return protocol;
}

std::size_t EthernetFrame::getMsgSize()
{
	return m_msg_size;
}

std::vector<char> EthernetFrame::getMsg()
{
	assert(m_data.size() >= 14);
	if (m_msg_size > 0) {
		return std::vector<char>(m_data.begin() + 14, m_data.begin() + 14 + m_msg_size);
	}
	else {
		return std::vector<char>();
	}
}

std::size_t EthernetFrame::getSize()
{
	return m_msg_size + 14;
}

char* EthernetFrame::getData()
{
	return m_data.data();
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
