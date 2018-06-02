//MIPFrame.hpp
//Author: Sivert Andresen Cubedo
#pragma once
#ifndef MIPFrame_HEADER
#define MIPFrame_HEADER

//C++
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <utility>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <bitset>

//Local
#include "AddressTypes.hpp"

class MIPFrame
{
public:
	/*
	const
	*/
	static const std::size_t FRAME_HEADER_SIZE = 18;	//in byte
	static const std::size_t ETH_HEADER_SIZE = 14;		//in byte
	static const std::size_t MIP_HEADER_SIZE = 4;		//in byte
	static const std::size_t FRAME_MAX_SIZE = 1514;		//in byte
	enum TRA 
	{
		ZERO =	0b000,
		T =		0b001,
		R =		0b010,
		A =		0b100
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
	Set ethernet dest.
	Parameters:
		dest		dest address
	Return:
		void
	*/
	void setEthDest(MACAddress & dest);

	/*
	Set ethernet Source.
	Parameters:
		source		source address
	Return:
		void
	*/
	void setEthSource(MACAddress & source);

	/*
	Set ethernet protocol.
	Parameters:
		protocol	protocol to set to
	Return:
		void
	*/
	void setEthProtocol(int protocol);

	/*
	Set mip TRA bits.
	Parameters:
		tra		uint8_t where the first 3 bits are relevant
	Return:
		void
	*/
	void setMipTRA(int tra);

	/*
	Set mip dest address-
	Parameters:
		dest	destination address
	Return:
		void
	*/
	void setMipDest(MIPAddress dest);

	/*
	Set mip source address.
	Parameters:
		source	source address
	Return:
		void
	*/
	void setMipSource(MIPAddress source);

	/*
	Set mip ttl bits.
	Parameters:
		ttl		int where the first 4 bits are relevant
	Return:
	*/
	void setMipTTL(int ttl);

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
		buf			pointer to msg
		buf_size	size of msg in buf
	Return:
		void
	*/
	void setMsg(const char* buf, std::size_t size);

	/*
	Set total size of frame.
	Parameters:
		size
	Return:
		void
	*/
	void setSize(std::size_t size);
	
	/*
	Get ethernet dest.
	Parameters:
	Return:
		dest
	*/
	MACAddress getEthDest();

	/*
	Get ethernet Source.
	Parameters:
	Return:
		source
	*/
	MACAddress getEthSource();

	/*
	Get ethernet protocol.
	Parameters:
	Return:
		protocol
	*/
	int getEthProtocol();

	/*
	Get mip tra.
	Parameters:
	Return:
		tra as int
	*/
	int getMipTRA();

	/*
	Get mip dest.
	Parameters:
	Return:
		dest
	*/
	MIPAddress getMipDest();

	/*
	Get mip source.
	Parameters:
	Return:
		source
	*/
	MIPAddress getMipSource();

	/*
	Get mip ttl.
	Parameters:
	Return:
		ttl as int
	*/
	int getMipTTL();

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
	char* getMsg();

	/*
	Get total size of frame (in bytes).
	Parameters:
	Return:
		size
	*/
	std::size_t getSize();

	/*
	Get raw buffer.
	Parameters:
	Return:
		pointer to raw data of frame
	*/
	char* getData();

	/*
	Exchanges underlying container with other frame.
	Parameters:
		other		ref to frame the swap
	Return:
		void
	*/
	void swap(MIPFrame & other);

	/*
	To string (for testing).
	Parameters:
	Return:
		string
	*/
	std::string toString();
private:
	std::vector<char> m_data;	//must be 18 byte or bigger
};

#endif // !MIPFrame_HEADER
