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
			c.cost = INF_COST;
		}
	}
}

void DistanceVectorTable::clear()
{
	m_data.clear();
}

void DistanceVectorTable::packAdvertisment(std::vector<char>& buf, MIPAddress dest_mip)
{
	buf.clear();
	for (Column & c : m_data) {
		if (c.via != dest_mip) {
			buf.push_back(c.to);
			buf.push_back(c.cost);
		}
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

bool DistanceVectorTable::update(MIPAddress sender, DistanceVectorTable & advert)
{
	bool changed = false;
	for (Column & advert_c : advert.m_data) {
		Cost cost;
		if (advert_c.cost < INF_COST) {
			cost = advert_c.cost + HOP_COST;
		}
		else {
			cost = INF_COST;
		}
		//auto table_it = std::find_if(m_data.begin(), m_data.end(), [&](Column & c) {return c.to == advert_c.to; });
		auto table_it = findTo(advert_c.to);
		if (table_it == m_data.end()) {
			add(advert_c.to, sender, cost);
			changed = true;
		}
		else if (cost < (*table_it).cost || (sender == (*table_it).via && cost != (*table_it).cost)) {
			(*table_it).via = sender;
			(*table_it).cost = cost;
			changed = true;
		}
	}
	return changed;
}

std::vector<DistanceVectorTable::Column>::iterator DistanceVectorTable::findTo(MIPAddress to)
{
	return std::find_if(m_data.begin(), m_data.end(), [&](Column & c) {return c.to == to; });
}

void DistanceVectorTable::addArpDiscovery(MIPAddress mip)
{
	auto it = findTo(mip);
	if (it == m_data.end()) {
		add(mip, mip, HOP_COST);
	}
	else {
		(*it).via = mip;
		(*it).cost = HOP_COST;
	}
}








