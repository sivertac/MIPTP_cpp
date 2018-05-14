//MIPFrame.hpp
//Author: Sivert Andresen Cubedo
#pragma once
#ifndef MIPFrame_HEADER
#define MIPFrame_HEADER

#include <vector>
#include <string>
#include <sstream>
#include <utility>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <bitset>

#include "AddressTypes.hpp"

class MIPFrame
{
public:
	enum TRA 
	{
		T = 0x01,
		R = 0x02,
		A = 0x04
	};

	/*
	Constructor.
	*/
	MIPFrame();

	/*
	Set TRA bits.
	Parameters:
		tra		uint8_t where the first 3 bits are relevant
	Return:
		void
	*/
	void setTRA(std::uint8_t tra);

	/*
	Set dest address-
	Parameters:
		dest	destination address
	Return:
		void
	*/
	void setDest(MIPAddress dest);

	/*
	Set source address.
	Parameters:
		source	source address
	Return:
		void
	*/
	void setSource(MIPAddress source);

	/*
	Set ttl bits.
	Parameters:
		ttl		uint8_t where the first 4 bits are relevant
	Return:
	*/
	void setTTL(std::uint8_t ttl);

	/*
	Set msg size.
	Parameters:
		size	new size
	Return:
		void
	*/
	void setMsgSize(std::size_t size);

	/*
	Set msg.
	Parameters:
		first	iterator
		last	iterator
	Return:
		void
	*/
	template <class InputIt>
	void setMsg(InputIt first, InputIt last)
	{
		assert(std::distance(first, last) < 1482);		//frame must be inside 1500 byte
		if (m_data.size() > 4) {
			m_data.resize(4);
		}
		m_data.insert(m_data.begin() + 4, first, last);
		setMsgSize(std::distance(first, last));
	}

private:
	std::size_t m_msg_size;
	std::vector<char> m_data;	//must be 4 byte or bigger
};

#endif // !MIPFrame_HEADER
