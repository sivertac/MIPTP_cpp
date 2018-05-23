//MIPFrame.cpp
//Author: Sivert Andresen Cubedo

#include "../include/MIPFrame.hpp"

MIPFrame::MIPFrame() :
	m_data(FRAME_HEADER_SIZE)
{
}

MIPFrame::MIPFrame(char* buf, std::size_t size)
{
	assert(size >= FRAME_HEADER_SIZE);
	if (m_data.size() != size) {
		m_data.resize(size);
	}
	std::memcpy(m_data.data(), buf, size);
}

void MIPFrame::setEthDest(MACAddress & dest)
{
	std::copy(dest.begin(), dest.end(), m_data.begin());
}

void MIPFrame::setEthSource(MACAddress & source)
{
	std::copy(source.begin(), source.end(), m_data.begin() + 6);
}

void MIPFrame::setEthProtocol(int protocol)
{
	std::memcpy(m_data.data() + 12, (char*)&protocol, 2);
}

void MIPFrame::setMipTRA(int tra)
{
	const unsigned long int mask = 0b111;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data() + ETH_HEADER_SIZE);
	//clear bits
	*header &= ~(1UL << 0);
	*header &= ~(1UL << 1);
	*header &= ~(1UL << 2);
	//insert tra
	*header |= ((tra & mask) << 0);
}

void MIPFrame::setMipDest(MIPAddress dest)
{
	const unsigned long int mask = 0b11111111;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data() + ETH_HEADER_SIZE);
	//clear bits
	*header &= ~(1UL << 3);
	*header &= ~(1UL << 4);
	*header &= ~(1UL << 5);
	*header &= ~(1UL << 6);
	*header &= ~(1UL << 7);
	*header &= ~(1UL << 8);
	*header &= ~(1UL << 9);
	*header &= ~(1UL << 10);
	//insert dest
	*header |= ((dest & mask) << 3);
}

void MIPFrame::setMipSource(MIPAddress source)
{
	const unsigned long int mask = 0b11111111;	//C++14
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data() + ETH_HEADER_SIZE);
	//clear bits
	*header &= ~(1UL << 11);
	*header &= ~(1UL << 12);
	*header &= ~(1UL << 13);
	*header &= ~(1UL << 14);
	*header &= ~(1UL << 15);
	*header &= ~(1UL << 16);
	*header &= ~(1UL << 17);
	*header &= ~(1UL << 18);
	//insert source
	*header |= ((source & mask) << 11);
}

void MIPFrame::setMipTTL(int ttl)
{
	const unsigned long int mask = 0b1111;	//C++14
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data() + ETH_HEADER_SIZE);
	//clear bits
	*header &= ~(1UL << 28);
	*header &= ~(1UL << 29);
	*header &= ~(1UL << 30);
	*header &= ~(1UL << 31);
	//insert ttl
	*header |= ((ttl & mask) << 28);
}

void MIPFrame::setMsgSize(std::size_t size)
{
	const unsigned long int mask = 0b111111111;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data() + ETH_HEADER_SIZE);
	std::size_t l, d;
	std::size_t payload_length;
	if (size > 0) {
		l = 4 + size;
		d = 4 - (l % 4);
		payload_length = (l + d) / 4;
	}
	else {
		payload_length = 0;
	}
	//clear bits
	*header &= ~(1UL << 19);
	*header &= ~(1UL << 20);
	*header &= ~(1UL << 21);
	*header &= ~(1UL << 22);
	*header &= ~(1UL << 23);
	*header &= ~(1UL << 24);
	*header &= ~(1UL << 25);
	*header &= ~(1UL << 26);
	*header &= ~(1UL << 27);
	//insert payload_lenght
	*header |= ((payload_length & mask) << 19);
	//resize vector if needed
	std::size_t current_msg_size = getMsgSize();
	if (m_data.size() - FRAME_HEADER_SIZE != current_msg_size) {
		m_data.resize(current_msg_size + FRAME_HEADER_SIZE);
	}
}

void MIPFrame::setMsg(const char* buf, std::size_t size)
{
	if (size != getMsgSize()) {
		setMsgSize(size);
	}
	if (size > 0) {
		std::memcpy(m_data.data() + FRAME_HEADER_SIZE, buf, size);
	}
}

void MIPFrame::setSize(std::size_t size)
{
	setMsgSize(size - ETH_HEADER_SIZE);
}

MACAddress MIPFrame::getEthDest()
{
	MACAddress dest;
	std::copy(m_data.begin(), m_data.begin() + 6, dest.begin());
	return dest;
}

MACAddress MIPFrame::getEthSource()
{
	MACAddress source;
	std::copy(m_data.begin() + 6, m_data.begin() + 12, source.begin());
	return source;
}

int MIPFrame::getEthProtocol()
{
	int protocol;
	std::memcpy((char*)&protocol, m_data.data() + 12, 2);
	return protocol;
}

int MIPFrame::getMipTRA()
{
	const unsigned long int mask = 0b111;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data() + ETH_HEADER_SIZE);
	int n;
	n = mask & (*header >> 0);
	return n;
}

MIPAddress MIPFrame::getMipDest()
{
	const unsigned long int mask = 0b11111111;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data() + ETH_HEADER_SIZE);
	MIPAddress dest = mask & (*header >> 3);
	return dest;
}

MIPAddress MIPFrame::getMipSource()
{
	const unsigned long int mask = 0b11111111;	//C++14
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data() + ETH_HEADER_SIZE);
	MIPAddress source = mask & (*header >> 11);
	return source;
}

int MIPFrame::getMipTTL()
{
	const unsigned long int mask = 0b1111;	//C++14
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data() + ETH_HEADER_SIZE);
	int ttl = mask & (*header >> 28);
	return ttl;
}

std::size_t MIPFrame::getMsgSize()
{
	const unsigned long int mask = 0b111111111;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data() + ETH_HEADER_SIZE);
	int n;
	n = mask & (*header >> 19);
	if (n > 0) {
		//if this then it is normal frame
		return static_cast<std::size_t>(n * 4 - 4);		//- 4 to remove header
	}
	else {
		//if this then the frame is a ARP or response frame
		return 0;
	}
}

char* MIPFrame::getMsg()
{
	return m_data.data() + FRAME_HEADER_SIZE;
}

std::string MIPFrame::toString()
{
	std::ostringstream ss;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	ss << "Header: " << std::bitset<32>(*header) << "\n"
		<< "TRA: " << getMipTRA() << "\n"
		<< "Dest: " << static_cast<std::size_t>(getMipDest()) << "\n"
		<< "Source: " << static_cast<std::size_t>(getMipSource()) << "\n"
		<< "TTL: " << getMipTTL() << "\n"
		<< "Msg size: " << getMsgSize();
	return ss.str();
}

std::size_t MIPFrame::getSize()
{
	return m_data.size();
}

char* MIPFrame::getData()
{
	return m_data.data();
}
