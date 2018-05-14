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

#include "AddressTypes.hpp"

class EthernetFrame
{
public:
	/*
	Constructor.
	*/
	EthernetFrame();

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
		first		iterator
		last		iterator
	Return:
		void
	*/
	template <class InputIt>
	void setMsg(InputIt first, InputIt last)
	{
		assert(std::distance(first, last) < 1486);		//frame must be inside 1500 byte
		if (m_data.size() > 14) {
			m_data.reserve(14);
		}
		m_data.insert(m_data.end(), first, last);
		setMsgSize(std::distance(first, last));
	}

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
	std::vector<char> getMsg();

	/*
	Get total size of frame (in bytes).
	Parameters:
	Return:
		size
	*/
	std::size_t getSize();

	/*
	To string (for testing).
	Parameters:
	Return:
		string
	*/
	std::string toString();
private:
	std::size_t m_msg_size;
	std::vector<char> m_data;	//must be atleast 14 bytes
};

#endif // !EthernetFrame_HEADER

