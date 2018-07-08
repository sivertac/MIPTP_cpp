//file_receiver.cpp
//Author: Sivert Andresen Cubedo

#include <iostream>
#include <fstream>
#include <utility>
#include <string>
#include <vector>
#include <array>

#include "../include/AddressTypes.hpp"
#include "../include/TransportInterface.hpp"
#include "../include/CrossIPC.hpp"

const char* help_string = "Args: ./file_receiver <transport_deamon sock> <file storage directory> <listen port>";
/*
Args: ./file_receiver <transport_deamon sock> <file storage directory> <listen port>
*/
int main(int argc, char** argv)
{
	//check args
	if (argc != 4) {
		std::cout << "Wrong args\n";
		std::cout << help_string << "\n";
		return EXIT_FAILURE;
	}
	std::string filepath(argv[2]);
	Port listen_port = std::atoi(argv[3]);

	/*
	Protocol:
	<name size: std::size_t> <name> <file size: std::size_t> <file>
	*/

	//listen
	AnonymousSocket sock;
	try {
		sock = TransportInterface::requestListen(argv[1], listen_port);
	}
	catch (TransportInterface::PortInUseException & e) {
		std::cerr << "file_sender: port in use\n";
		return EXIT_FAILURE;
	}
	catch (TransportInterface::TimeoutException & e) {
		std::cerr << "file_sender: listen timeout\n";
		return EXIT_FAILURE;
	}

	std::size_t total_bytes_received = 0;
	std::size_t buffer_size;
	std::vector<char> buffer;

	//receive filename
	total_bytes_received += sock.readGenericUntil(buffer_size);
	buffer.resize(buffer_size);	
	total_bytes_received += sock.readUntil(buffer.data(), buffer_size);
	std::string filename(buffer.begin(), buffer.end());
	
	//receive file
	total_bytes_received += sock.readGenericUntil(buffer_size);
	buffer.resize(buffer_size);
	std::size_t ret = 0;
	while (ret < buffer_size) {
		ret += sock.read(buffer.data() + ret, buffer_size - ret);
		
		std::cout << "file_receiver: " << ret << "/" << buffer_size << "\n";
	}

	total_bytes_received += ret;

	sock.closeResources();

	std::cout << "file_receiver: transmission done\n";

	std::cout << "file_receiver: filename: " << filename << "\n";
	std::cout << "file_receiver: total_bytes_received: " << total_bytes_received << "\n";

	//write file
	if (filepath.find('/') == filepath.npos) {
		filepath = filename;
	}
	else {
		filepath = filepath.substr(0, filepath.find_last_of('/') + 1);
		filepath += filename;
	}
	std::cout << "file_receiver: filepath: " << filepath << "\n";
	std::fstream file_stream(filepath, file_stream.binary | file_stream.out | file_stream.trunc);
	if (!file_stream.is_open()) {
		std::cerr << "file_receiver: failed to open/create: " << filepath << "\n";
		return EXIT_FAILURE;
	}
	file_stream.write(buffer.data(), buffer.size());

	std::cout << "file_receiver: terminating\n";

	return 0;
}
