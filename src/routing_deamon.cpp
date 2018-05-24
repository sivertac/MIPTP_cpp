//routing_deamon.cpp
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

#include "../include/AddressTypes.hpp"
#include "../include/CrossIPC.hpp"
#include "../include/EventPoll.hpp"

/*
Globals
*/
AnonymousSocket update_sock;
AnonymousSocket lookup_sock;

/*
Receive on update_sock.
Parameters:
Return:
	void
Global:
	update_sock
*/
void receiveUpdateSock()
{
	//std::uint8_t option;
	
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

}

/*
args: ./routinh_deamon <update sock> <lookup sock>
*/
int main(int argc, char** argv)
{
	//parse args
	if (argc < 3) {
		std::cerr << "Too few args\n";
		return EXIT_FAILURE;
	}
	
	update_sock = AnonymousSocket(argv[1]);
	lookup_sock = AnonymousSocket(argv[2]);

	
	EventPoll epoll;

	epoll.addFriend<AnonymousSocket>(update_sock);
	epoll.addFriend<AnonymousSocket>(lookup_sock);

	while (epoll.wait() == EventPoll::Okay) {
		for (auto & ev : epoll.m_event_vector) {
			int in_fd = ev.data.fd;
			
			if (in_fd == update_sock.getFd()) {

			}
			else if (in_fd == lookup_sock.getFd()) {

			}
		}
	}

	std::cout << "routing_deamon is terminating\n";

	return 0;
}

