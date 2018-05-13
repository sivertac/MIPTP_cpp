//AddressTypes.hpp
//Author: Sivert Andresen Cubedo
#pragma once
#ifndef AddressTypes_HEADER
#define AddressTypes_HEADER

#include <cstdint>
#include <array>

/*
MIP address type.
*/
using MIPtype = std::uint8_t;

/*
MAC address type.
*/
using MACtype = std::array<std::uint8_t, 6>;

#endif // !AddressTypes_HEADER

