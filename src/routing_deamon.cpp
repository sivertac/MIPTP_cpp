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

/*
Globals
*/
CrossIPC::AnonymousSocket update_sock;
CrossIPC::AnonymousSocket lookup_sock;

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
	
	update_sock = CrossIPC::AnonymousSocket(argv[1]);
	lookup_sock = CrossIPC::AnonymousSocket(argv[2]);

	std::string str = update_sock.readString();


	std::cout << "From routing_deamon: " << str << "\n";
	
	return 0;
}

