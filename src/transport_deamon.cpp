//transport_deamon.cpp
//Author: Sivert Andresen Cubedo

//C++
#include <iostream>
#include <cstdio>
#include <string>

//LINUX
#include <signal.h>

//Local
#include "../include/LinuxException.hpp"
#include "../include/AddressTypes.hpp"
#include "../include/CrossIPC.hpp"
#include "../include/CrossForkExec.hpp"
#include "../include/EventPoll.hpp"
#include "../include/Application.hpp"

/*
Globals
*/
ChildProcess mip_deamon;
EventPoll epoll;
AnonymousSocket transport_sock;
NamedSocket application_sock;

/*
transport protocol:
	ApplicationServer is listening on Port p1.
	ApplicationClient sends connect request from Port p2 to ApplicationServer on p1.
	ApplicationServer replies accept if connection is accepted or not.

2 way communication is possible.
	
packet layout:
	header is always 48 bits long.
	<packet type: 2 bits> <padding bits: 2 bits> <dest port: 14 bits> <source port: 14 bits> <packet sequence number: 16 bits>
	
	packet type:
		Can be: 
			01 : connect request
			10 : connect reply
			11 : data transmission
*/

/*
Send on transport_sock.
Parameters:
	dest
	msg
Return:
	void
Global:
	transport_sock
*/
void sendTransportSock(MIPAddress dest, std::vector<char> & msg)
{
	//send
	//	dest
	//	msg size
	//	msg
	std::size_t msg_size = msg.size();
	transport_sock.write(reinterpret_cast<char*>(&dest), sizeof(dest));
	transport_sock.write(reinterpret_cast<char*>(&msg_size), sizeof(msg_size));
	transport_sock.write(reinterpret_cast<char*>(msg.data()), msg_size);
}

/*
Receive on application_sock.
Parameters:
Return:
	void
Global:
	application_sock
*/
void receiveApplicationSock()
{

}

/*
Receive on transport_sock.
Parameters:
Return:
	void
Global:
	transport_sock
*/
void receiveTransportSock()
{
	//receive
	//	source
	//	msg size
	//	msg
	MIPAddress source_mip;
	std::size_t msg_size;
	static std::vector<char> msg;
	transport_sock.read(reinterpret_cast<char*>(&source_mip), sizeof(source_mip));
	transport_sock.read(reinterpret_cast<char*>(&msg_size), sizeof(msg_size));
	if (msg.size() != msg_size) {
		msg.resize(msg_size);
	}
	transport_sock.read(msg.data(), msg_size);

	//test
	std::cout << std::string(msg.data()) << "\n";
}

/*
Signal function.
*/
void sigintHandler(int signum)
{
	epoll.closeResources();
}

/*
args: ./transport_deamon <socket_application> <timeout> [mip addresses]
*/
int main(int argc, char** argv)
{
	//parse args
	if (argc < 4) {
		std::cerr << "Too few args\n";
		return EXIT_FAILURE;
	}

	//signal
	struct sigaction sa;
	sa.sa_handler = &sigintHandler;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		throw LinuxException::Error("sigaction()");
	}

	//Connect MIP_deamon
	{
		auto pair = AnonymousSocket::createAnonymousSocketPair();
		std::vector<std::string> args(1);
		args[0] = pair.second.toString();
		for (int i = 3; i < argc; ++i) {
			args.push_back(argv[i]);
		}
		mip_deamon = ChildProcess::forkExec("./MIP_deamon", args);
		transport_sock = pair.first;
		pair.second.closeResources();
	}

	//socket application
	{
		std::string name(argv[1]);
		application_sock = NamedSocket(name);
	}

	//epoll
	epoll = EventPoll(100);

	epoll.addFriend<AnonymousSocket>(transport_sock);
	epoll.addFriend<NamedSocket>(application_sock);

	while (epoll.wait(20) == EventPoll::Okay) {
		for (auto & ev : epoll.m_event_vector) {
			int in_fd = ev.data.fd;
			if (in_fd == transport_sock.getFd()) {
				receiveTransportSock();
			}
			else if (in_fd == application_sock.getFd()) {
				receiveApplicationSock();
			}
		}
	}

	std::cout << "transport_deamon joining mip_deamon\n";

	transport_sock.closeResources();
	application_sock.closeResources();

	mip_deamon.join();

	std::cout << "transport_deamon terminating\n";

	return 0;
}



