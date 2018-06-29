//test_client.cpp
//Author: Sivert Andresen Cubedo

#include <iostream>
#include <string>

#include "../include/TransportInterface.hpp"

const char* help_string = "Args: ./test_client <transport_deamon sock> <target address> <port>";
/*
Args: ./test_client <transport_deamon sock> <target address> <port>
*/
int main(int argc, char** argv)
{
	if (argc != 4) {
		std::cout << "Wrong args\n";
		std::cout << help_string << "\n";
		return EXIT_FAILURE;
	}

	AnonymousSocket sock = TransportInterface::requestConnect(argv[1], std::atoi(argv[2]), std::atoi(argv[3]));

	std::cout << "test_client: connected\n";

	sock.closeResources();
	
	return 0;
}


