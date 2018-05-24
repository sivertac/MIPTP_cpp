//DistanceVectorTable.cpp
//Author: Sivert Andresen Cubedo

#include "../include/DistanceVectorTable.hpp"

void DistanceVectorTable::add(MIPAddress to, MIPAddress via, Cost cost)
{
	Column c;
	c.to = to;
	c.via = via;
	c.cost = cost;
	m_data.push_back(c);
}

void DistanceVectorTable::setViaInfinity(MIPAddress via)
{
	for (Column & c : m_data) {
		if (c.via == via) {
			c.cost = INFINITY;
		}
	}
}

void DistanceVectorTable::clear()
{
	m_data.clear();
}

void DistanceVectorTable::packAdvertisment(std::vector<char>& buf)
{
	buf.clear();
	for (Column & c : m_data) {
		buf.push_back(c.to);
		buf.push_back(c.cost);
	}
}

void DistanceVectorTable::unpackAdvertisment(std::vector<char>& buf)
{
	std::size_t size = buf.size() / 2;
	std::size_t i = 0;
	if (m_data.size() != size) {
		m_data.resize(size);
	}
	for (Column & c : m_data) {
		c.to = buf.data()[i];
		c.cost = buf.data()[i + 1];
		i += 2;
	}
}
