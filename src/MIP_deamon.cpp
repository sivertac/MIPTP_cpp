//MIP_deamon.cpp
//Author: Sivert Andresen Cubedo

//C++
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
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
#include "../include/MIPFrame.hpp"
#include "../include/CrossIPC.hpp"
#include "../include/CrossForkExec.hpp"

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
std::vector<ARPPair> arp_pair_vec;					//(can't have duplicates)
ChildProcess routing_deamon;
AnonymousSocket update_sock;
AnonymousSocket lookup_sock;

/*
Print arp_pair_vec.
Parameters:
Return:
	void
Global:
	arp_pair_vec
*/
void printARPTable()
{
	std::printf("%s: %4s:\n", "MIP", "MAC");
	for (auto & p : arp_pair_vec) {
		std::printf("%d: %4x:%x:%x:%x:%x:%x\n", (int)p.mip, p.mac[0], p.mac[1], p.mac[2], p.mac[3], p.mac[4], p.mac[5]);
	}
}

/*
Add pair to arp_pair_vec.
Parameters:
	sock		ref to sock that Node was discovered on
	mip			mip
	mac			mac
Return:
	void
Global:
	arp_pair_vec
*/
void addARPPair(RawSock::MIPRawSock & sock, MIPAddress mip, MACAddress mac)
{
	ARPPair pair;
	pair.mip = mip;
	pair.mac = mac;
	pair.sock = &sock;
	arp_pair_vec.push_back(pair);
}

/*
Send response frame to pair.
Parameters:
	pair		ref to pair
Return:
	void
*/
void sendResponseFrame(ARPPair & pair)
{
	static MIPFrame mip_frame;
	mip_frame.setMipTRA(MIPFrame::ZERO);
	mip_frame.setMipDest(pair.mip);
	mip_frame.setMipSource(pair.sock->getMip());
	mip_frame.setMsgSize(0);
	mip_frame.setMipTTL(0xff);
	
	MACAddress dest = pair.mac;
	MACAddress source = pair.sock->getMac();
	mip_frame.setEthDest(dest);
	mip_frame.setEthSource(source);
	mip_frame.setEthProtocol(htons(RawSock::MIPRawSock::ETH_P_MIP));
	mip_frame.setMsg(mip_frame.getData(), mip_frame.getSize());
	pair.sock->sendMipFrame(mip_frame);
}

/*
Send broadcast frame on sock.
Parameters:
	sock		ref to sock
Return:
	void
*/
void sendBroadcastFrame(RawSock::MIPRawSock & sock)
{
	static MIPFrame mip_frame;
	mip_frame.setMipTRA(MIPFrame::A);
	mip_frame.setMipDest(0xff);
	mip_frame.setMipSource(sock.getMip());
	mip_frame.setMsgSize(0);
	mip_frame.setMipTTL(0xff);

	static MACAddress dest{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	MACAddress source = sock.getMac();
	mip_frame.setEthDest(dest);
	mip_frame.setEthSource(source);
	mip_frame.setEthProtocol(htons(RawSock::MIPRawSock::ETH_P_MIP));
	mip_frame.setMsg(mip_frame.getData(), mip_frame.getSize());
	sock.sendMipFrame(mip_frame);
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
	static MIPFrame mip_frame;
	sock.recvMipFrame(mip_frame);
	int tra = mip_frame.getMipTRA();
	MIPAddress mip = mip_frame.getMipSource();
	std::vector<ARPPair>::iterator arp_it;
	switch (tra)
	{
	case MIPFrame::A:
		arp_it = std::find_if(arp_pair_vec.begin(), arp_pair_vec.end(), [&](ARPPair & p) { return p.mip == mip; });
		if (arp_it == arp_pair_vec.end()) {
			addARPPair(sock, mip_frame.getMipSource(), mip_frame.getEthSource());
			sendResponseFrame(arp_pair_vec.back());
		}
		else {
			sendResponseFrame((*arp_it));
		}
		break;
	case MIPFrame::ZERO:
		arp_it = std::find_if(arp_pair_vec.begin(), arp_pair_vec.end(), [&](ARPPair & p) { return p.mip == mip; });
		if (arp_it == arp_pair_vec.end()) {
			addARPPair(sock, mip_frame.getMipSource(), mip_frame.getEthSource());
		}
		break;
	default:
		throw std::runtime_error("Invalid TRA");
		break;
	}
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
	
	//Connect transport_deamon

	//Connect routing_deamon
	{
		auto pair1 = AnonymousSocket::createAnonymousSocketPair();
		auto pair2 = AnonymousSocket::createAnonymousSocketPair();
		
		std::vector<std::string> args(2);
		args[0] = pair1.second.toString();
		args[1] = pair2.second.toString();
		routing_deamon = ChildProcess::forkExec("./routing_deamon", args);

		update_sock = pair1.first;
		lookup_sock = pair2.first;
		pair1.second.closeResources();
		pair2.second.closeResources();
	}

	update_sock.writeString("fuck you");
	
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

	//Send first broadcast frames
	for (auto & s : raw_sock_vec) {
		sendBroadcastFrame(s);
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

	update_sock.closeResources();
	lookup_sock.closeResources();

	routing_deamon.join();
	
	return EXIT_SUCCESS;
}

