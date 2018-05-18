//MIP_deamon.cpp
//Author: Sivert Andresen Cubedo

//C++
#include <iostream>
#include <string>
#include <vector>
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

/*
Globals
*/
std::vector<RawSock::MIPRawSock> raw_sock_vec;

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
			return;
		}
		std::string & name = inter_names.back();
		if (name == "lo") {
			continue;
		}
		raw_sock_vec.push_back(RawSock::MIPRawSock(name, mip));
		inter_names.pop_back();
	}
}

/*
Signal function.
*/
void sigintHandler(int signum)
{
	//transport_deamon will handle this
	std::cout << "Hello from signal\n";
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

	std::cout << "raw_sock_vec.size(): " << raw_sock_vec.size() << "\n";
	
	return EXIT_SUCCESS;
}

