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
#include <queue>

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
#include "../include/TimerWrapper.hpp"

enum update_sock_option 
{
	LOCAL_MIP = 1,
	ARP_DISCOVERY = 2,
	ARP_LOSTCONNECTION = 3,
	ADVERTISEMENT = 4
};

struct ARPPair
{
	bool reply;						//if true then pair is not lost
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
EventPoll epoll;
AnonymousSocketPacket transport_sock;
AnonymousSocket update_sock;
AnonymousSocket lookup_sock;
std::queue<std::pair<bool, MIPFrame>> lookup_queue;	//pair <true if source mip is set, frame>
const int ARP_TIMEOUT = 2000;						//in ms

/*
Send local mip discovery to update_sock.
Parameters:
	mip		local mip discovered
Return:
	void
Global:
	update_sock
*/
void sendLocalMip(MIPAddress mip)
{
	std::uint8_t option = update_sock_option::LOCAL_MIP;
	update_sock.write(reinterpret_cast<char*>(&option), sizeof(option));
	update_sock.write(reinterpret_cast<char*>(&mip), sizeof(mip));
}

/*
Send arp discovery with update_sock.
Parameters:
	mip		discovered address
Return:
	void
Glboal:
	update_sock
*/
void sendArpDiscovery(MIPAddress mip)
{
	std::uint8_t option = update_sock_option::ARP_DISCOVERY;
	update_sock.write(reinterpret_cast<char*>(&option), sizeof(option));
	update_sock.write(reinterpret_cast<char*>(&mip), sizeof(mip));
}

/*
Send arp lost connection with update_sock.
Parameters:
	mip		lost address
Return:
	void
Global:
	update_sock
*/
void sendArpLostConnection(MIPAddress mip)
{
	std::uint8_t option = update_sock_option::ARP_LOSTCONNECTION;
	update_sock.write(reinterpret_cast<char*>(&option), sizeof(option));
	update_sock.write(reinterpret_cast<char*>(&mip), sizeof(mip));
}

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
	pair.reply = true;
	pair.mip = mip;
	pair.mac = mac;
	pair.sock = &sock;
	arp_pair_vec.push_back(pair);

	printARPTable();		//debug
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
Receive on transport_sock.
Parameters:
Return:
	void
Global:
	transport_sock
	lookup_queue
*/
void receiveTransportSock()
{
	static MIPFrame frame;
	if (frame.getMsgSize() != MIPFrame::MSG_MAX_SIZE) {
		frame.setMsgSize(MIPFrame::MSG_MAX_SIZE);
	}
	
	MIPAddress dest;
	std::size_t msg_size;

	AnonymousSocketPacket::IovecWrapper<3> iov;
	iov.setIndex(0, dest);
	iov.setIndex(1, msg_size);
	iov.setIndex(2, frame.getMsg(), frame.getMsgSize());

	transport_sock.recviovec(iov);

	if (frame.getMsgSize() != msg_size) {
		frame.setMsgSize(msg_size);
	}

	frame.setMipTRA(MIPFrame::T);
	frame.setMipDest(dest);
	frame.setMipTTL(0xff);
	frame.setEthProtocol(htons(RawSock::MIPRawSock::ETH_P_MIP));
	lookup_queue.emplace(false, frame);
	lookup_sock.write(reinterpret_cast<char*>(&dest), sizeof(dest));

	std::cout << "MIP_deamon receive transport_sock\n";
}

/*
Receive on lookup_sock.
Parameters:
Return:
	void
Global:
	lookup_sock
*/
void receiveLookupSock()
{
	bool success;
	lookup_sock.read(reinterpret_cast<char*>(&success), sizeof(success));
	if (success) {
		MIPAddress via;
		lookup_sock.read(reinterpret_cast<char*>(&via), sizeof(via));
		auto pair_it = std::find_if(arp_pair_vec.begin(), arp_pair_vec.end(), [&](ARPPair & p) { return p.mip == via; });
		if (pair_it != arp_pair_vec.end()) {
			ARPPair pair = (*pair_it);
			MIPFrame & frame = lookup_queue.front().second;
			if (!lookup_queue.front().first) {
				frame.setMipSource(pair.sock->getMip());
			}
			MACAddress eth_dest = pair.mac;
			MACAddress eth_source = pair.sock->getMac();
			frame.setEthDest(eth_dest);
			frame.setEthSource(eth_source);
			frame.setEthProtocol(htons(RawSock::MIPRawSock::ETH_P_MIP));
			pair.sock->sendMipFrame(frame);

			std::cout << "MIP_deamon sending transport frame to mip: " << (int)frame.getMipDest() << "\n";
		}
		lookup_queue.pop();
	}
	else {
		lookup_queue.pop();
	}
}

/*
Receive on update_sock.
Parameters:
Return:
	void
Global:
	update_sock
	arp_pair_vec
*/
void receiveUpdateSock()
{
	//receive:
	//	to
	//	ad size
	//	ad
	static MIPFrame frame;
	MIPAddress dest_mip;
	std::size_t ad_size;
	update_sock.read(reinterpret_cast<char*>(&dest_mip), sizeof(dest_mip));
	update_sock.read(reinterpret_cast<char*>(&ad_size), sizeof(ad_size));
	frame.setMsgSize(ad_size);
	update_sock.read(frame.getMsg(), ad_size);
	auto pair_it = std::find_if(arp_pair_vec.begin(), arp_pair_vec.end(), [&](ARPPair & p) { return p.mip == dest_mip; });
	//if (pair_it == arp_pair_vec.end()) {
	//	throw std::runtime_error("Should have found pair");
	//}
	if (pair_it != arp_pair_vec.end()) {
		ARPPair & pair = (*pair_it);
		frame.setMipTRA(MIPFrame::R);
		frame.setMipDest(pair.mip);
		frame.setMipSource(pair.sock->getMip());
		frame.setMipTTL(0xff);
		MACAddress eth_dest = pair.mac;
		MACAddress eth_source = pair.sock->getMac();
		frame.setEthDest(eth_dest);
		frame.setEthSource(eth_source);
		frame.setEthProtocol(htons(RawSock::MIPRawSock::ETH_P_MIP));
		pair.sock->sendMipFrame(frame);
		//std::cout << "Send ad on raw sock:\n";
		//std::cout << "ad_size: " << ad_size << "\n";
		//std::cout << "ad_length field: " << (int)*reinterpret_cast<std::uint16_t*>(frame.getMsg()) << "\n";;
		//std::cout << "frame msg size: " << frame.getMsgSize() << "\n";
	}
}

/*
Receive on raw sock.
Parameters:
	sock		ref to sock
Return:
	void
Global:
	update_sock
*/
void receiveRawSock(RawSock::MIPRawSock & sock)
{
	static MIPFrame mip_frame;
	sock.recvMipFrame(mip_frame);
	int tra = mip_frame.getMipTRA();
	MIPAddress source_mip = mip_frame.getMipSource();
	MIPAddress dest_mip = mip_frame.getMipDest();
	std::uint8_t option;
	std::size_t msg_size = mip_frame.getMsgSize();
	std::vector<ARPPair>::iterator arp_it;
	switch (tra)
	{
	case MIPFrame::A:
		arp_it = std::find_if(arp_pair_vec.begin(), arp_pair_vec.end(), [&](ARPPair & p) { return p.mip == source_mip; });
		if (arp_it == arp_pair_vec.end()) {
			addARPPair(sock, mip_frame.getMipSource(), mip_frame.getEthSource());
			sendArpDiscovery(source_mip);
			sendResponseFrame(arp_pair_vec.back());
		}
		else {
			arp_it->reply = true;
			sendResponseFrame((*arp_it));
		}
		break;
	case MIPFrame::ZERO:
		arp_it = std::find_if(arp_pair_vec.begin(), arp_pair_vec.end(), [&](ARPPair & p) { return p.mip == source_mip; });
		if (arp_it == arp_pair_vec.end()) {
			addARPPair(sock, mip_frame.getMipSource(), mip_frame.getEthSource());
			sendArpDiscovery(source_mip);
		}
		else {
			arp_it->reply = true;
		}
		break;
	case MIPFrame::R:
		//send ad to routing_deamon
		//send:
		//	from
		//	ad size
		//	ad
		option = update_sock_option::ADVERTISEMENT;
		update_sock.write(reinterpret_cast<char*>(&option), sizeof(std::uint8_t));
		update_sock.write(reinterpret_cast<char*>(&source_mip), sizeof(source_mip));
		update_sock.write(reinterpret_cast<char*>(&msg_size), sizeof(msg_size));
		update_sock.write(mip_frame.getMsg(), msg_size);
		break;
	case MIPFrame::T:
		//cache frame in lookup_queue
		//send:
		//	mip

		std::cout << "MIP_deamon received tranport frame from mip: "<< (int)mip_frame.getMipSource() << "\n";

		if (std::find_if(raw_sock_vec.begin(), raw_sock_vec.end(), [&](RawSock::MIPRawSock & s) { return s.getMip() == dest_mip; }) == raw_sock_vec.end()) {
			int ttl = mip_frame.getMipTTL();
			if (ttl <= 0) {
				//drop frame
				std::cout << "ttl 0, dropping frame\n";
			}
			else {
				mip_frame.setMipTTL(ttl - 1);
				lookup_queue.emplace(true, mip_frame);
				lookup_sock.write(reinterpret_cast<char*>(&dest_mip), sizeof(dest_mip));
			}
		}
		else {
			//send to transport deamon
			AnonymousSocketPacket::IovecWrapper<3> iov;
			iov.setIndex(0, source_mip);
			iov.setIndex(1, msg_size);
			iov.setIndex(2, mip_frame.getMsg(), msg_size);
			transport_sock.sendiovec(iov);
			//transport_sock.write(reinterpret_cast<char*>(&source_mip), sizeof(source_mip));
			//transport_sock.write(reinterpret_cast<char*>(&msg_size), sizeof(msg_size));
			//transport_sock.write(mip_frame.getMsg(), msg_size);
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
	epoll.closeResources();
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
	
	//signal
	struct sigaction sa;
	sa.sa_handler = &sigintHandler;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		throw LinuxException::Error("sigaction()");
	}

	//Connect transport_deamon
	transport_sock = AnonymousSocketPacket(argv[1]);

	//Connect routing_deamon
	{
		auto pair1 = AnonymousSocket::createPair();
		auto pair2 = AnonymousSocket::createPair();
		
		std::vector<std::string> args(2);
		args[0] = pair1.second.toString();
		args[1] = pair2.second.toString();
		routing_deamon = ChildProcess::forkExec("./routing_deamon", args);

		update_sock = pair1.first;
		lookup_sock = pair2.first;
		pair1.second.closeResources();
		pair2.second.closeResources();
	}

	//Make MIPRawSock
	{
		std::vector<MIPAddress> mip_vec;
		for (int i = 2; i < argc; ++i) {
			mip_vec.push_back(std::atoi(argv[i]));
		}
		initRawSock(mip_vec);
	}
	
	//epoll
	epoll = EventPoll(100);
	for (auto & sock : raw_sock_vec) {
		epoll.addFriend<RawSock::MIPRawSock>(sock);
	}
	epoll.addFriend<AnonymousSocketPacket>(transport_sock);
	epoll.addFriend<AnonymousSocket>(update_sock);
	epoll.addFriend<AnonymousSocket>(lookup_sock);

	//send local mip addresses to routing_deamon
	for (auto & s : raw_sock_vec) {
		sendLocalMip(s.getMip());
	}

	//Send first broadcast frames
	for (auto & s : raw_sock_vec) {
		sendBroadcastFrame(s);
	}

	//timer
	TimerWrapper timeout_timer;
	
	timeout_timer.setExpirationFromNow(ARP_TIMEOUT);
	epoll.addFriend(timeout_timer);

	while (!epoll.isClosed()) {
		try {
			auto & event_vec = epoll.wait(20);		//<- epoll exceptions will happen here
			for (auto & ev : event_vec) {
				int in_fd = ev.data.fd;
				//check
				if (in_fd == timeout_timer.getFd()) {
					//std::cout << "Expired: " << timeout_timer.readExpiredTime() << "\n";
					for (auto it = arp_pair_vec.begin(); it != arp_pair_vec.end(); ) {
						if (!it->reply) {
							sendArpLostConnection(it->mip);
							arp_pair_vec.erase(it);
						}
						else {
							it->reply = false;
							++it;
						}
					}
					for (RawSock::MIPRawSock & sock : raw_sock_vec) {
						sendBroadcastFrame(sock);
					}
					timeout_timer.setExpirationFromNow(ARP_TIMEOUT);
				}
				else if (in_fd == transport_sock.getFd()) {
					receiveTransportSock();
				}
				else if (in_fd == update_sock.getFd()) {
					receiveUpdateSock();
				}
				else if (in_fd == lookup_sock.getFd()) {
					receiveLookupSock();
				}
				else {
					//check raw
					auto raw_it = std::find_if(raw_sock_vec.begin(), raw_sock_vec.end(), [&](RawSock::MIPRawSock & s){ return s.getFd() == in_fd; });
					if (raw_it != raw_sock_vec.end()) {
						receiveRawSock((*raw_it));
					}
				}
			}
		}
		catch (LinuxException::InterruptedException & e) {
			//if this then interrupted
			std::cout << "MIP_deamon: epoll interrupted\n";
		}
	}

	update_sock.closeResources();
	lookup_sock.closeResources();

	routing_deamon.join();
	
	return EXIT_SUCCESS;
}

