//main.cpp
//Author: Sivert Andresen Cubedo

//C++
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <functional>
#include <thread>

//Local
#include "../include/MIPFrame.hpp"
#include "../include/MIPTPFrame.hpp"
#include "../include/Application.hpp"
#include "../include/AddressTypes.hpp"
#include "../include/CrossIPC.hpp"

AnonymousSocketPacket sock1;
AnonymousSocketPacket sock2;

void child_handler()
{
	std::size_t size;
	std::vector<char> vec(100);
	AnonymousSocketPacket::IovecWrapper<2> iov;
	iov.setIndex(0, size);
	iov.setIndex(1, vec.data(), 100);

	sock2.recviovec(iov);

	std::cout << size << "\n";
	std::cout << std::string(vec.begin(), vec.begin() + size) << "\n";

	std::cout << "child thread exit\n";
}

int main(int argc, char** argv)
{
	auto pair = AnonymousSocketPacket::createPair();
	sock1 = pair.first;
	sock2 = pair.second;

	std::thread child_thread(child_handler);
	
	std::string str = "this is a string";

	std::size_t size;
	std::vector<char> vec;

	vec.insert(vec.end(), str.begin(), str.end());
	size = vec.size();

	AnonymousSocketPacket::IovecWrapper<2> iov;
	iov.setIndex(0, size);
	iov.setIndex(1, vec.data(), size);

	sleep(1);

	sock1.sendiovec(iov);

	child_thread.join();

	std::cout << "main thread exit\n";

	return 0;
}

/*
int main(int argc, char** argv)
{
	std::queue<std::pair<MIPAddress, MIPTPFrame>> queue;
	std::vector<char> buffer;
	auto pair = AnonymousSocket::createAnonymousSocketPair();
	pair.second.enableNonBlock();

	SlidingWindow::SendWindow<10> send(queue, pair.second, 10, 1, 2);
	SlidingWindow::ReceiveWindow<10> receive(queue, buffer, 20, 2, 1);

	send.queueFrames();

	std::cout << "queue.size(): " << queue.size() << "\n";

	std::cout << &send << "\n";
	std::cout << &receive << "\n";

	return 0;
}
*/

/*
int main(int argc, char** argv)
{	
	if (argc > 1) {
		std::cout << "Hello from child\n";
		CrossIPC::AnonymousSocket parent_sock(argv[1]);
		
		std::string str1("This is str1 from child\n");
		std::cout << "Child sending msg from parent, size: " << str1.size() << "str:" << str1 << "\n";
		parent_sock.writeString(str1);

		std::cout << "Child waiting for reply\n";
		std::string str2 = parent_sock.readString();
		std::cout << "Child received msg from parent, size: " << str2.size() << "str:" << str2 << "\n";
		
		std::cout << "Child termenating\n";
		parent_sock.closeResources();

		return 0;
	}
	else {
		std::cout << "Hello from parent\n";
		
		std::cout << "Parent creating anon sock pair\n";
		auto pair = CrossIPC::createAnonymousSocketPair();
		auto & child_sock = pair.first;

		std::cout << "Parent spawning child\n";
		std::string program_path("./main");
		std::vector<std::string> program_args{ pair.second.toString() };
		auto child = CrossForkExec::forkExec(program_path, program_args);
		pair.second.closeResources();

		std::cout << "Parent waiting for msg from child\n";
		std::string str1 = child_sock.readString();
		std::cout << "Parent received msg from parent, size: " << str1.size() << "str:" << str1 << "\n";
		

		std::string str2("This is str1 from child\n");
		std::cout << "Parent sending msg from child, size: " << str2.size() << "str:" << str2 << "\n";
		child_sock.writeString(str2);


		std::cout << "Parent joining child\n";
		child.join();

		std::cout << "Parent terminating\n";
		child_sock.closeResources();
		child.closeResources();

		return 0;
	}
}
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