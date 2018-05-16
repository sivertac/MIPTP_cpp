//RawSock.hpp
//Author: Sivert Andresen Cubedo
#pragma once
#ifndef RawSock_HEADER
#define RawSock_HEADER

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <array>
#include <exception>
#include <algorithm>

#include <string.h>

//platform spesific
#ifdef WINDOWS
//windows stuff
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms740548(v=vs.85).aspx
#include <Winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>		//NEED TO LINK Iphlpapi.lib
#include <stdio.h>
#include <Windows.h>
#elif LINUX
//unix stuff
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <ifaddrs.h>
#else
//error
#error RawSock.hpp: Not defined target OS
#endif

//local
#include "EthernetFrame.hpp"

namespace RawSock
{
	static const int ETH_P_MIP = 0x88B5;

	class MIPRawSock
	{
	public:
		/*
		Constructor.
		Parameters:
			interface_name		name of interface
			mip					mip address
		*/
		MIPRawSock(const std::string & interface_name, MIPAddress mip);

		/*
		Close socket (release resources).
		Parameters:
		Return:
		*/
		void closeResources();

		/*
		Send Ethernet frame with socket (this will not modify the frame).
		Parameters:
			frame		frame to send
		Return:
		*/
		void sendEthernetFrame(EthernetFrame & frame);

		/*
		Receive Ethernet frame with socket.
		Parameters:
		Return:
			received frame
		*/
		EthernetFrame recvEthernetFrame();

	private:
		int m_fd;
		MIPAddress m_mip;
		MACAddress m_mac;
		struct sockaddr_ll m_sock_address;
	};

	/*
	Get network interface names on this host.
	Parameters:
		family_filter		vector containing what kind of interfaces to extract
	Return:
		Vector containing names of interfaces
	*/
	std::vector<std::string> getInterfaceNames(const std::vector<int> & family_filter);
	//std::vector<std::wstring> getInterfaceNames();		//windows uses wide char in kernel :/

	/*
	Get MAC address of interface.
	Parameters:
		fd					socket fd
		interface_name		name in string
	Return:
		MACtype
	*/
	MACAddress getMacAddress(int fd, const std::string & interface_name);

	/*
	Set socket to nonblocking.
	Parameters:
		fd					file descriptor
	Return:
		void
	*/
	void setNonBlocking(int fd);

}

#endif // !RawSock_HEADER
