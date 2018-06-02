//MIPTPFrame.cpp
//Author: Sivert Andresen Cubedo

#include "../include/MIPTPFrame.hpp"

MIPTPFrame::MIPTPFrame() :
	m_data(FRAME_HEADER_SIZE)
{
}

MIPTPFrame::MIPTPFrame(char * buf, std::size_t size)
{
	assert(size >= FRAME_HEADER_SIZE);
	if (m_data.size() != size) {
		m_data.resize(size);
	}
	std::memcpy(m_data.data(), buf, size);
}

void MIPTPFrame::setType(int type)
{
	const unsigned long int mask = 0b11;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	*header &= ~(mask << 0);
	*header |= ((type & mask) << 0);
}

void MIPTPFrame::setPadding(int padding)
{
	const unsigned long int mask = 0b11;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	*header &= ~(mask << 2);
	*header |= ((padding & mask) << 2);
}

void MIPTPFrame::setDest(Port dest)
{
	const unsigned long int mask = 0b11111111111111;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	*header &= ~(mask << 4);
	*header |= ((dest & mask) << 4);
}

void MIPTPFrame::setSource(Port source)
{
	const unsigned long int mask = 0b11111111111111;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	*header &= ~(mask << 18);
	*header |= ((source & mask) << 18);
}

void MIPTPFrame::setSequenceNumber(int seq)
{
	std::memcpy(m_data.data() + 4, reinterpret_cast<char*>(&seq), 2);
}

void MIPTPFrame::setMsgSize(std::size_t size)
{
	m_data.resize(size + FRAME_HEADER_SIZE);
}

void MIPTPFrame::setMsg(const char * buf, std::size_t size)
{
	if (size != getMsgSize()) {
		setMsgSize(size);
	}
	std::memcpy(m_data.data() + FRAME_HEADER_SIZE, buf, size);
}

void MIPTPFrame::setSize(std::size_t size)
{
	setMsgSize(size - FRAME_HEADER_SIZE);
}

int MIPTPFrame::getType()
{
	const unsigned long int mask = 0b11;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	int type = mask & (*header >> 0);
	return type;
}

int MIPTPFrame::getPadding()
{
	const unsigned long int mask = 0b11;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	int padding = mask & (*header >> 2);
	return padding;
}

Port MIPTPFrame::getDest()
{
	const unsigned long int mask = 0b11111111111111;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	Port dest = mask & (*header >> 4);
	return dest;
}

Port MIPTPFrame::getSource()
{
	const unsigned long int mask = 0b11111111111111;
	std::uint32_t* header = reinterpret_cast<std::uint32_t*>(m_data.data());
	Port source = mask & (*header >> 18);
	return source;
}

int MIPTPFrame::getSequenceNumber()
{
	int seq;
	std::memcpy(reinterpret_cast<char*>(&seq), m_data.data() + 4, 2);
	return seq;
}

std::size_t MIPTPFrame::getMsgSize()
{
	return m_data.size() - FRAME_HEADER_SIZE;
}

char* MIPTPFrame::getMsg()
{
	return m_data.data() + FRAME_HEADER_SIZE;
}

std::size_t MIPTPFrame::getSize()
{
	return m_data.size();
}

char * MIPTPFrame::getData()
{
	return m_data.data();
}

void MIPTPFrame::swap(MIPTPFrame & other)
{
	m_data.swap(other.m_data);
}

std::string MIPTPFrame::toString()
{
	std::ostringstream ss;
	ss << "Header: " << std::bitset<48>(m_data.data()) << "\n"
		<< "Type: " << getType() << "\n"
		<< "Padding: " << getPadding() << "\n"
		<< "Dest: " << static_cast<int>(getDest()) << "\n"
		<< "Source: " << static_cast<int>(getSource()) << "\n"
		<< "Seq: " << getSequenceNumber() << "\n"
		<< "Msg size: " << getMsgSize();
	return ss.str();
}
