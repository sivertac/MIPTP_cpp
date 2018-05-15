//MIPFrame.hpp
//Author: Sivert Andresen Cubedo
#pragma once
#ifndef MIPFrame_HEADER
#define MIPFrame_HEADER

#include <iostream>
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
		ZERO = 0b000,
		T = 0b001,
		R = 0b010,
		A = 0b100
	};

	/*
	Constructor.
	*/
	MIPFrame();

	/*
	Constructor.
	Construct from buffer.
	*/
	MIPFrame(char* buf, std::size_t size);

	/*
	Get raw buffer.
	Parameters:
	Return:
	vector holding buffer
	*/
	std::vector<char> getRawBuffer();

	/*
	Set TRA bits.
	Parameters:
		tra		uint8_t where the first 3 bits are relevant
	Return:
		void
	*/
	void setTRA(int tra);

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
		ttl		int where the first 4 bits are relevant
	Return:
	*/
	void setTTL(int ttl);

	/*
	Set msg size (size will be rounded up to a number that is divisible by 4.
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

	/*
	Get tra.
	Parameters:
	Return:
		tra as int
	*/
	int getTRA();

	/*
	Get dest.
	Parameters:
	Return:
		dest
	*/
	MIPAddress getDest();

	/*
	Get source.
	Parameters:
	Return:
		source
	*/
	MIPAddress getSource();

	/*
	Get ttl.
	Parameters:
	Return:
		ttl as int
	*/
	int getTTL();

	/*
	Get msg size.
	Parameters:
	Return:
		size in bytes
	*/
	std::size_t getMsgSize();

	/*
	Get msg.
	Parameters:
	Return:
		msg
	*/
	std::vector<char> getMsg();

	/*
	To string (for testing).
	Parameters:
	Return:
		string
	*/
	std::string toString();
private:
	std::vector<char> m_data;	//must be 4 byte or bigger
};

#endif // !MIPFrame_HEADER
