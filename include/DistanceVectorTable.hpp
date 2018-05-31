//DistanceVectorTable.hpp
//Author: Sivert Andresen Cubedo

#pragma once
#ifndef DistanceVectorTable_HEADER
#define DistanceVectorTable_HEADER

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <cstring>
#include <mutex>

//local
#include "AddressTypes.hpp"

class DistanceVectorTable
{
public:
	using Cost = std::uint8_t;
	struct Column
	{
		MIPAddress to;
		MIPAddress via;
		Cost cost;
	};

	/*
	const
	*/
	static const Cost INF_COST = 0xff;
	static const Cost HOP_COST = 1;

	/*
	Set cost of all entrys that has via set to 'via' to infinity.
	Parameters:
		via
	Return:
		void
	*/
	void setViaInfinity(MIPAddress via);

	/*
	Pack advertisment for transmission.
	Parameters:
		buf
		dest_mip
	Return:
		void
	*/
	void packAdvertisment(std::vector<char> & buf, MIPAddress dest_mip);

	/*
	Unpack advertisment.
	Parameters:
		buf
	Return:
		void
	*/
	void unpackAdvertisment(std::vector<char> & buf);

	/*
	Update table with advertisment from sender.
	Parameters:
		sender		address of sender of advertisment
		advert		ref to table containing advertisment that is from sender
	Return:
		true if the table was changed
		false if not
	*/
	bool update(MIPAddress sender, DistanceVectorTable & advert);

	/*
	Find Column with 'to'.
	Parameters:
		c		ref to column to fill if we find one
		to		
	Return:
		true if Column exits
		false if Column doesn't exist
	*/
	bool findTo(Column & c, MIPAddress to);

	/*
	Add local mip to table.
	Parameters:
		mip
	Return:
		void
	*/
	void addLocalMip(MIPAddress mip);

	/*
	Add arp discovery to table.
	Parameters:
		mip		discovery
	Return:
		void
	*/
	void addArpDiscovery(MIPAddress mip);

	/*
	To string.
	Parameters:
	Return:
		string of table
	*/
	std::string toString();

private:
	/*
	Add entry to table.
	Parameters:
		to
		via
		cost
	Return:
		void
	*/
	void add(MIPAddress to, MIPAddress via, Cost cost);

	std::vector<Column> m_data;		//container
	std::mutex m_mutex;				//mutex for m_data
};

#endif // !DistanceVectorTable_HEADER
