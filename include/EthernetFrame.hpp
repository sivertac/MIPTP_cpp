//EthernetFrame.hpp
//Author: Sivert Andresen Cubedo
#pragma once

#ifndef EthernetFrame_HEADER
#define EthernetFrame_HEADER

#include <vector>
#include <string>
#include <sstream>
#include <utility>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cstddef>

#include "AddressTypes.hpp"

class EthernetFrame
{
public:
	/*
	Constructor.
	*/
	EthernetFrame();

	/*
	Constructor.
	Construct from buffer.
	*/
	EthernetFrame(char* buf, std::size_t size);

	/*
	Set dest.
	Parameters:
		dest		dest address
	Return:
		void
	*/
	void setDest(MACAddress & dest);

	/*
	Set Source.
	Parameters:
		source		source address
	Return:
		void
	*/
	void setSource(MACAddress & source);

	/*
	Set protocol.
	Parameters:
		protocol	protocol to set to
	Return:
		void
	*/
	void setProtocol(int protocol);

	/*
	Set total size of frame (for managing heap alloc).
	Parameters:
	Return:
		void
	*/
	void setSize(std::size_t size);

	/*
	Set msg size.
	Parameters:
		size		size to set to
	Return:
		void
	*/
	void setMsgSize(std::size_t size);

	/*
	Set msg.
	Parameters:
		buf
		buf_size
	Return:
		void
	*/
	void setMsg(const char* buf, std::size_t size);

	/*
	Get dest.
	Parameters:
	Return:
		dest
	*/
	MACAddress getDest();

	/*
	Get Source.
	Parameters:
	Return:
		source
	*/
	MACAddress getSource();

	/*
	Get protocol.
	Parameters:
	Return:
		protocol
	*/
	int getProtocol();

	/*
	Get msg size.
	Parameters:
	Return:
		size
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
	Get ref to vector of frame.
	Parameters:
	Return:
		ref to vec
	*/
	std::vector<char> & getVector();

	/*
	To string (for testing).
	Parameters:
	Return:
		string
	*/
	std::string toString();

	/*
	const
	*/
	static const std::size_t FRAME_HEADER_SIZE = 14;	//in byte
	static const std::size_t FRAME_MAX_SIZE = 1514;		//in byte
private:
	std::size_t m_msg_size;
	std::vector<char> m_data;	//must be atleast 14 bytes
};

#endif // !EthernetFrame_HEADER

