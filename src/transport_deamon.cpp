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

/*
Globals
*/
ChildProcess mip_deamon;
AnonymousSocket transport_sock;

/*
Signal function.
*/
void sigintHandler(int signum)
{
	
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

	std::cout << "transport_deamon joining mip_deamon\n";

	mip_deamon.join();

	std::cout << "transport_deamon terminating\n";

	return 0;
}