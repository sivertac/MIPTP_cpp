//MIPFrame.cpp
//Author: Sivert Andresen Cubedo

#include "../include/MIPFrame.hpp"

MIPFrame::MIPFrame() :
	m_msg_size(0),
	m_data(4)
{
}

void MIPFrame::setTRA(std::uint8_t tra)
{
	const std::uint8_t mask = 0b11100000;
	std::bitset<32> header(reinterpret_cast<std::uint32_t*>(m_data.data()));
	header.reset(0);
	header.reset(1);
	header.reset(2);
	header |= (tra & mask);
	auto temp = header.to_ulong();
	std::memcpy(m_data.data(), &temp, 4);
}
