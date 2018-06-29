//test_server.cpp
//Author: Sivert Andresen Cubedo

#include <iostream>
#include <string>
#include <vector>

#include "../include/TransportInterface.hpp"

const char* help_string = "Args: ./test_server <transport_deamon sock> <port>";
/*
Args: ./test_server <transport_deamon sock> <port>
*/
int main(int argc, char** argv)
{
	if (argc != 3) {
		std::cout << "Wrong args\n";
		std::cout << help_string << "\n";
		return EXIT_FAILURE;
	}

	AnonymousSocket sock = TransportInterface::requestListen(argv[1], std::atoi(argv[2]));

	std::cout << "test_server: connected\n";

	//try to read from sock

	std::size_t msg_size;
	
	
	sock.readGeneric(msg_size);
	std::cout << "test_server: msg_size " << msg_size << "\n";

	std::vector<char> msg(msg_size);

	std::size_t ret = sock.read(msg.data(), msg_size);

	std::cout << "test_server: ret " << ret << "\n";
	std::cout << "test_server: " << std::string(msg.data()) << "\n";

	sock.closeResources();

	std::cout << "test_server: terminating\n";

	return 0;
}

