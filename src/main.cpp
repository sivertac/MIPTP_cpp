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
#include "../include/MIPFrame.hpp"


std::string msgToString(std::vector<char> & msg)
{
	std::string str;
	str.insert(str.end(), msg.begin(), msg.end());
	std::cout << "msg.size(): " << msg.size() << "\n";
	std::cout << "str.size(): " << str.size() << "\n";
	return str;
}

int main(int argc, char** argv)
{	
	MIPFrame frame;

	frame.setTRA(MIPFrame::T);
	std::cout << frame.toString() << "\n";
	
	frame.setTRA(MIPFrame::R);
	frame.setDest(0xff);
	std::cout << frame.toString() << "\n";

	frame.setTRA(MIPFrame::A);
	frame.setSource(0xff);
	std::cout << frame.toString() << "\n";

	frame.setTRA(MIPFrame::ZERO);
	frame.setDest(0);
	frame.setSource(0);
	frame.setTTL(0b1001);
	std::cout << frame.toString() << "\n";

	frame.setTTL(0);
	frame.setMsgSize(11);
	std::cout << frame.toString() << "\n";

	std::string str1("Hello this is str1");
	std::cout << "str1.size(): " << str1.size() << "\n";
	frame.setMsg(str1.begin(), str1.end());
	auto msg = frame.getMsg();
	std::cout << frame.toString() << "\n";
	std::cout << msgToString(msg) << "\n";

	/*
	EthernetFrame frame;
	
	MACAddress mac1{ 100, 200, 32, 0, 1, 2 };
	MACAddress mac2{ 1, 2, 3, 4, 5, 6 };

	frame.setDest(mac1);
	frame.setSource(mac2);
	frame.setProtocol(42);
	
	std::string str1("Hello, this is a message");
	std::string str2("Hello, this is the second message");

	std::cout << str1.size() << "\n";

	frame.setMsg(str1.begin(), str1.end());

	std::cout << frame.toString() << "\n";

	auto msg = frame.getMsg();

	std::cout << msgToString(msg) << "\n";

	//str2
	std::cout << str2.size() << "\n";

	frame.setMsg(str2.begin(), str2.end());

	std::cout << frame.toString() << "\n";

	msg = frame.getMsg();

	std::cout << msgToString(msg) << "\n";
	*/


	return 0;
}
