//MIPFrame.cpp
//Author: Sivert Andresen Cubedo

#include "../include/MIPFrame.hpp"

MIPFrame::MIPFrame() :
	m_data(4)
{
}

MIPFrame::MIPFrame(char * buf, std::size_t size)
{
	assert(size >= 4);
	if (m_data.size() != size) {
		m_data.resize(size);
	}
	std::memcpy(m_data.data(), buf, size);
}

std::vector<char> MIPFrame::getRawBuffer()
{
	return std::vector<char>(m_data);
}

void MIPFrame::setTRA(int tra)
{
	const unsigned long int mask = 0b111;	//C++14
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	//clear bits
	*header &= ~(1UL << 0);
	*header &= ~(1UL << 1);
	*header &= ~(1UL << 2);
	//insert tra
	*header |= ((tra & mask) << 0);
}

void MIPFrame::setDest(MIPAddress dest)
{
	const unsigned long int mask = 0b11111111;	//C++14
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
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

void MIPFrame::setSource(MIPAddress source)
{
	const unsigned long int mask = 0b11111111;	//C++14
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
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

void MIPFrame::setTTL(int ttl)
{
	const unsigned long int mask = 0b1111;	//C++14
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
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
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
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
	if (m_data.size() - 4 != current_msg_size) {
		m_data.resize(current_msg_size + 4);
	}
}

void MIPFrame::setMsg(const char* buf, std::size_t size)
{
	if (size != getMsgSize()) {
		setMsgSize(size);
	}
	if (size > 0) {
		std::memcpy(m_data.data() + 4, buf, size);
	}
}

int MIPFrame::getTRA()
{
	const unsigned long int mask = 0b111;	//C++14
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	int n;
	n = mask & (*header >> 0);
	return n;
}

MIPAddress MIPFrame::getDest()
{
	const unsigned long int mask = 0b11111111;	//C++14
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	MIPAddress dest = mask & (*header >> 3);
	return dest;
}

MIPAddress MIPFrame::getSource()
{
	const unsigned long int mask = 0b11111111;	//C++14
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	MIPAddress source = mask & (*header >> 11);
	return source;
}

int MIPFrame::getTTL()
{
	const unsigned long int mask = 0b1111;	//C++14
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	int ttl = mask & (*header >> 28);
	return ttl;
}

std::size_t MIPFrame::getMsgSize()
{
	const unsigned long int mask = 0b111111111;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
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
	return m_data.data() + 4;
}



std::string MIPFrame::toString()
{
	std::ostringstream ss;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	ss << "Header: " << std::bitset<32>(*header) << "\n"
		<< "TRA: " << getTRA() << "\n"
		<< "Dest: " << static_cast<std::size_t>(getDest()) << "\n"
		<< "Source: " << static_cast<std::size_t>(getSource()) << "\n"
		<< "TTL: " << getTTL() << "\n"
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
