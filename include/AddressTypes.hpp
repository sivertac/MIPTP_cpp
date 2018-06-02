//AddressTypes.hpp
//Author: Sivert Andresen Cubedo
#pragma once
#ifndef AddressTypes_HEADER
#define AddressTypes_HEADER

//C++
#include <cstdint>
#include <array>
#include <string>
#include <sstream>

/*
MIP address type.
*/
using MIPAddress = std::uint8_t;
std::string toStringMIPAddress(const MIPAddress mip);


/*
MAC address type.
*/
using MACAddress = std::array<std::uint8_t, 6>;
std::string toStringMACAddress(const MACAddress mac);

/*
Port address type.
*/
using Port = std::uint16_t;
std::string toStringPort(const Port port);

#endif // !AddressTypes_HEADER

