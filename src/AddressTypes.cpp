//AddressTypes.cpp
//Author: Sivert Andresen Cubedo

#include "../include/AddressTypes.hpp"

std::string toStringMIPAddress(const MIPAddress & mip)
{
	std::ostringstream ss;
	ss << static_cast<std::size_t>(mip);
	return ss.str();
}

std::string toStringMACAddress(const MACAddress & mac)
{
	std::ostringstream ss;
	for (auto it = mac.begin(); it != mac.end() - 1; ++it) {
		ss << std::hex << static_cast<std::size_t>((*it)) << ':';
	}
	ss << std::hex << static_cast<std::size_t>(mac.back());
	return ss.str();
}


