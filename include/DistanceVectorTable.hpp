//DistanceVectorTable.hpp
//Author: Sivert Andresen Cubedo

#pragma once
#ifndef DistanceVectorTable_HEADER
#define DistanceVectorTable_HEADER

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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
	static const Cost INFINITY = 0xff;

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

	/*
	Set cost of all entrys that has via set to 'via' to infinity.
	Parameters:
		via
	Return:
		void
	*/
	void setViaInfinity(MIPAddress via);

	/*
	Clear all elements.
	Parameters:
	Return:
		void
	*/
	void clear();

	/*
	Pack advertisment for transmission.
	Parameters:
		buf
	Return:
		void
	*/
	void packAdvertisment(std::vector<char> & buf);

	/*
	Unpack advertisment.
	Parameters:
		buf
	Return:
		void
	*/
	void unpackAdvertisment(std::vector<char> & buf);

private:
	std::vector<Column> m_data;
};

#endif // !DistanceVectorTable_HEADER
