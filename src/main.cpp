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



int main(int argc, char** argv)
{	
	



	return 0;
}

/*
std::string msgToString(std::vector<char> & msg)
{
std::string str;
str.insert(str.end(), msg.begin(), msg.end());
std::cout << "msg.size(): " << msg.size() << "\n";
std::cout << "str.size(): " << str.size() << "\n";
return str;
}
*/

/*
MIPFrame mip_frame1;
MIPFrame mip_frame2;

std::string str1 = "this is str1";
std::string str2 = "this is str2";
mip_frame1.setMsg(str1.begin(), str1.end());

std::cout << "mip_frame1\n";
std::cout << mip_frame1.toString() << "\n";

auto raw = mip_frame1.getRawBuffer();

mip_frame2 = MIPFrame(raw.data(), raw.size());

std::cout << "mip_frame2\n";
std::cout << mip_frame2.toString() << "\n";

EthernetFrame ethernet_frame1;
EthernetFrame ethernet_frame2;

ethernet_frame1.setMsg(str2.begin(), str2.end());

MACAddress mac1{ 1, 2, 3, 4, 5, 6 };
ethernet_frame1.setDest(mac1);

std::cout << "ethernet_frame1\n";
std::cout << ethernet_frame1.toString() << "\n";

raw = ethernet_frame1.getRawBuffer();

ethernet_frame2 = EthernetFrame(raw.data(), raw.size());

std::cout << "ethernet_frame2\n";
std::cout << ethernet_frame2.toString() << "\n";
*/

/*
std::string msgToString(std::vector<char> & msg)
{
std::string str;
str.insert(str.end(), msg.begin(), msg.end());
std::cout << "msg.size(): " << msg.size() << "\n";
std::cout << "str.size(): " << str.size() << "\n";
return str;
}
*/

/*
MIPFrame frame;

frame.setTRA(MIPFrame::T);
std::cout << frame.toString() << "\n";

frame.setTRA(MIPFrame::R);
frame.setDest(0xff);
frame.setTTL(frame.getTTL() + 1);
std::cout << frame.toString() << "\n";

frame.setTRA(MIPFrame::A);
frame.setSource(0xff);
std::cout << frame.toString() << "\n";

frame.setTRA(MIPFrame::ZERO);
frame.setDest(0);
frame.setSource(0);
frame.setTTL(frame.getTTL() + 1);
std::cout << frame.toString() << "\n";

frame.setTTL(frame.getTTL() + 1);
frame.setMsgSize(11);
std::cout << frame.toString() << "\n";

std::string str1("Hello this is str1");
std::string str2("Hello this is str2____PADDING");
std::cout << "str1.size(): " << str1.size() << "\n";

frame.setMsg(str1.begin(), str1.end());
auto msg = frame.getMsg();
frame.setTTL(frame.getTTL() + 1);
std::cout << frame.toString() << "\n";
std::cout << msgToString(msg) << "\n";

frame.setMsg(str2.begin(), str2.end());
msg = frame.getMsg();
frame.setTTL(frame.getTTL() + 1);
std::cout << frame.toString() << "\n";
std::cout << msgToString(msg) << "\n";
*/

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