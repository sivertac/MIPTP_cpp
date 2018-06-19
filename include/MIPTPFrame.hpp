//MIPTPFrame.hpp
//Author: Sivert Andresen Cubedo
#pragma once
#ifndef MIPTPFrame_HEADER
#define MIPTPFrame_HEADER

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
/*
packet layout:
	header is always 48 bits long.
	<packet type : 2 bits> <padding bits : 2 bits> <dest port : 14 bits> <source port: 14 bits> <packet sequence number : 16 bits> <data>
	
	packet type :
		Can be :
			00 : connect request 
			01 : connect reply
			10 : data transmission
			11 : sack
*/
class MIPTPFrame
{
public:
	/*
	const
	*/
	static const std::size_t FRAME_HEADER_SIZE	= 6;									//in byte
	static const std::size_t FRAME_MAX_SIZE		= 1496;									//in byte
	static const std::size_t FRAME_MAX_MSG_SIZE = FRAME_MAX_SIZE - FRAME_HEADER_SIZE;	//in byte
	enum Packet_Type
	{
		request = 0b00,
		reply	= 0b01,
		data	= 0b10,
		sack	= 0b11
	};

	/*
	Constructor.
	*/
	MIPTPFrame();

	/*
	Constructor.
	Parameters:
		buf
		size
	*/
	MIPTPFrame(char* buf, std::size_t size);

	/*
	Set packet type.
	Parameters:
		type		int where the first 2 bits are relevant 
	Return:
		void
	*/
	void setType(int type);

	/*
	Set padding bits.
	Parameters:
		padding		int where the first 2 bits are relevant
	Return:
		void
	*/
	void setPadding(int padding);

	/*
	Set dest port.
	Parameters:
		dest
	Return:
		void
	*/
	void setDest(Port dest);

	/*
	Set source port.
	Parameters:
		source
	Return:
		void
	*/
	void setSource(Port source);

	/*
	Set sequence number.
	Parameters:
		num		num where the first 16 bits are relevant
	Return:
		void
	*/
	void setSequenceNumber(int seq);

	/*
	Set msg size.
	Parameters:
		size
	Return:
		void
	*/
	void setMsgSize(std::size_t size);

	/*
	Set msg.
	Parameters:
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
	Get type.
	Parameters:
	Return:
		type		int where the 2 first bits are relevant
	*/
	int getType();

	/*
	Get padding.
	Parameters:
	Return:
		padding		int where the first 2 bits are relevant
	*/
	int getPadding();

	/*
	Get dest port.
	Parameters:
	Return:
		dest
	*/
	Port getDest();

	/*
	Get source port.
	Parameters:
	Return:
		source
	*/
	Port getSource();

	/*
	Get sequence number.
	Parameters:
	Return:
		sequence number
	*/
	int getSequenceNumber();

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
		msg as raw pointer to msg
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
	void swap(MIPTPFrame & other);

	/*
	Calculate padding.
	Parameters:
		size
	Return
		padding		int where the first 2 bits are relevant
	*/
	static int calcPadding(std::size_t size);

	/*
	To string (for testing).
	Parameters:
	Return:
		string
	*/
	std::string toString();
private:
	std::vector<char> m_data;		//must be 6 byte or bigger
};

#endif // !MIPTPFrame_HEADER
