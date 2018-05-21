//MIP_deamon.cpp
//Author: Sivert Andresen Cubedo

//C++
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <stdexcept>

//LINUX
#include <signal.h>

//Local
#include "../include/LinuxException.hpp"
#include "../include/EventPoll.hpp"
#include "../include/RawSock.hpp"
#include "../include/AddressTypes.hpp"
#include "../include/EthernetFrame.hpp"
#include "../include/MIPFrame.hpp"

struct ARPPair
{
	MIPAddress mip;					//dest mip
	MACAddress mac;					//dest mac
	RawSock::MIPRawSock* sock;		//socket to reach dest
};

/*
Globals
*/
std::vector<RawSock::MIPRawSock> raw_sock_vec;		//(must keep order intact so not to invalidate ARPPairs)
std::vector<ARPPair> arp_pair_vec;

/*
Add pair to arp_pair_vec.
Parameters:
Return:
*/

/*
Send broadcast frame on sock.
Parameters:
	sock		ref to sock
Return:
*/
void sendBroadcastFrame(RawSock::MIPRawSock & sock)
{
	MIPFrame mip_frame;
	mip_frame.setTRA(MIPFrame::A);
	mip_frame.setDest(0xff);
	mip_frame.setSource(sock.getMip());
	mip_frame.setMsgSize(0);
	mip_frame.setTTL(0xff);
	
	EthernetFrame eth_frame;
	MACAddress dest{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	MACAddress source = sock.getMac();
	eth_frame.setDest(dest);
	eth_frame.setSource(source);
	eth_frame.setProtocol(htons(RawSock::MIPRawSock::ETH_P_MIP));
	eth_frame.setMsg(mip_frame.getData(), mip_frame.getSize());
	sock.sendEthernetFrame(eth_frame);
}

/*
Receive on raw sock.
Parameters:
	sock		ref to sock
Return:
	void
*/
void receiveRawSock(RawSock::MIPRawSock & sock)
{
	EthernetFrame frame;
	sock.recvEthernetFrame(frame);
	std::cout << frame.toString() << "\n";
	std::cout << frame.getMsg() << "\n";
}

/*
Init raw_sock_vec.
Parameters:
	mip_vec		ref to vec containing mip
Return:
	void
*/
void initRawSock(const std::vector<MIPAddress> & mip_vec)
{
	auto inter_names = RawSock::getInterfaceNames(std::vector<int>{ AF_PACKET });
	for (auto & mip : mip_vec) {
		if (inter_names.empty()) {
			
			//debug
			std::cout << raw_sock_vec.size() << " interfaces detected.\n";
			
			return;
		}
		std::string & name = inter_names.back();
		if (name == "lo") {
			continue;
		}
		raw_sock_vec.push_back(RawSock::MIPRawSock(name, mip));

		//debug
		std::cout << raw_sock_vec.back().toString() << "\n";

		inter_names.pop_back();
	}
	
	//debug
	std::cout << raw_sock_vec.size() << " interfaces detected.\n";
}

/*
Signal function.
*/
void sigintHandler(int signum)
{
	//transport_deamon will handle this
}

/*
args: ./MIP_deamon <transport connection> [mip addresses ...]
*/
int main(int argc, char** argv)
{
	//parse args
	if (argc < 3) {
		std::cerr << "Too few args\n";
		return EXIT_FAILURE;
	}
	
	//Connect transport deamon
	//TODO
	
	//Make MIPRawSock
	{
		std::vector<MIPAddress> mip_vec;
		for (int i = 2; i < argc; ++i) {
			mip_vec.push_back(std::atoi(argv[i]));
		}
		initRawSock(mip_vec);
	}

	//signal
	struct sigaction sa;
	sa.sa_handler = &sigintHandler;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		throw LinuxException::Error("sigaction()");
	}
	
	//epoll
	EventPoll epoll;
	
	for (auto & sock : raw_sock_vec) {
		epoll.addFriend<RawSock::MIPRawSock>(sock);
	}

	//test
	for (auto & s : raw_sock_vec) {
		EthernetFrame frame;

		MACAddress dest{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
		MACAddress source = s.getMac();
		std::string msg("fuck you");

		frame.setDest(dest);
		frame.setSource(source);
		frame.setProtocol(htons(RawSock::MIPRawSock::ETH_P_MIP));

		auto p = msg.c_str();
		frame.setMsg(p, msg.size());

		s.sendEthernetFrame(frame);
	}

	while (epoll.wait() == EventPoll::Okay) {
		for (auto & ev : epoll.m_event_vector) {
			int in_fd = ev.data.fd;
			//check raw
			auto raw_it = std::find_if(raw_sock_vec.begin(), raw_sock_vec.end(), [&](RawSock::MIPRawSock & s){ return s.getFd() == in_fd; });
			if (raw_it != raw_sock_vec.end()) {
				receiveRawSock((*raw_it));
			}
		}
	}
	
	return EXIT_SUCCESS;
}

