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
	std::lock_guard<std::mutex> lock(m_mutex);
	for (Column & c : m_data) {
		if (c.via == via) {
			c.cost = INF_COST;
		}
	}
}

void DistanceVectorTable::packAdvertisment(std::vector<char>& buf, MIPAddress dest_mip)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	//size 2 byte
	//ad
	std::uint16_t size;
	buf.clear();
	buf.resize(sizeof(size));
	for (Column & c : m_data) {
		if (c.via != dest_mip) {
			buf.push_back(c.to);
			buf.push_back(c.cost);
			size += 1;
		}
	}
	std::memcpy(buf.data(), &size, sizeof(size));
}

void DistanceVectorTable::unpackAdvertisment(std::vector<char>& buf)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	std::uint16_t size;
	std::memcpy(&size, buf.data(), sizeof(size));
	if (m_data.size() != size) {
		m_data.resize(size);
	}
	std::size_t i = sizeof(size);
	for (Column & c : m_data) {
		c.to = buf.data()[i];
		c.cost = buf.data()[i + 1];
		i += 2;
	}
}

bool DistanceVectorTable::update(MIPAddress sender, DistanceVectorTable & advert)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	bool changed = false;
	for (Column & advert_c : advert.m_data) {
		Cost cost;
		if (advert_c.cost < INF_COST) {
			cost = advert_c.cost + HOP_COST;
		}
		else {
			cost = INF_COST;
		}
		auto table_it = std::find_if(m_data.begin(), m_data.end(), [&](Column & c) {return c.to == advert_c.to; });
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

bool DistanceVectorTable::findTo(Column & c, MIPAddress to)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = std::find_if(m_data.begin(), m_data.end(), [&](Column & c) {return c.to == to; });
	if (it != m_data.end()) {
		c = (*it);
		return true;
	}
	else {
		return false;
	}
}

void DistanceVectorTable::addLocalMip(MIPAddress mip)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	add(mip, mip, 0);
}

void DistanceVectorTable::addArpDiscovery(MIPAddress mip)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = std::find_if(m_data.begin(), m_data.end(), [&](Column & c) {return c.to == mip; });
	if (it == m_data.end()) {
		add(mip, mip, HOP_COST);
	}
	else {
		(*it).via = mip;
		(*it).cost = HOP_COST;
	}
}

std::string DistanceVectorTable::toString()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	std::ostringstream ss;
	ss << std::left << std::setw(6) << "To:" << std::left << std::setw(6) << "Via:" << std::left << std::setw(6) << "Cost:";
	for (Column & p : m_data) {
		ss << "\n";
		ss << std::left << std::setw(6) << (int)p.to << std::left << std::setw(6) << (int)p.via << std::left << std::setw(6) << (int)p.cost;
	}
	return ss.str();
}








