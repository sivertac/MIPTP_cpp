//main.cpp
//Author: Sivert Andresen Cubedo

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

//local
//#include "../include/RawSock.hpp"			//Uses winsock2.h must be before stuff that uses Windows.h //NEED TO LINK Iphlpapi.lib
#include "../include/CrossIPC.hpp"
#include "../include/CrossForkExec.hpp"
#include "../include/EthernetFrame.hpp"


int main(int argc, char** argv)
{	
	EthernetFrame frame;
	
	MACAddress mac1{ 100, 200, 32, 0, 1, 2 };
	MACAddress mac2{ 1, 2, 3, 4, 5, 6 };

	frame.setDest(mac1);
	frame.setSource(mac2);
	frame.setProtocol(42);
	
	std::string str1("Hello, this is a message");

	frame.setMsg(str1.begin(), str1.end());

	std::cout << frame.toString() << "\n";



	return 0;
}
