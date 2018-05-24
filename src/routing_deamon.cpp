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

#include "../include/CrossIPC.hpp"
#include "../include/EventPoll.hpp"

/*
Globals
*/
AnonymousSocket update_sock;
AnonymousSocket lookup_sock;

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


	while (epoll.wait() == EventPoll::Okay) {
		for (auto & ev : epoll.m_event_vector) {
			int in_fd = ev.data.fd;
			
			std::cout << in_fd << "\n";
		}
	}


	return 0;
}

