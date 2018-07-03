//file_sender.cpp
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

/*
Extract last leaf from file path.
Parameters:
	path	file path
Return:
	file name
*/
std::string extractPathLeaf(std::string & path)
{
	return path.substr(path.find_last_of('/') + 1);
}

const char* help_string = "Args: ./file_sender <transport_deamon sock> <file path> <mip address> <port>";
/*
Args: ./file_sender <transport_deamon sock> <file path> <mip address> <port>
*/
int main(int argc, char** argv)
{
	//check args
	if (argc != 5) {
		std::cout << "Wrong args\n";
		std::cout << help_string << "\n";
		return EXIT_FAILURE;
	}
	std::string filepath(argv[2]);
	MIPAddress receiver_mip = std::atoi(argv[3]);
	Port receiver_port = std::atoi(argv[4]);

	std::streamsize chunk_size = 0;
	std::array<char, 14960> chunk_buffer;
	std::vector<char> buffer;
	std::fstream in_stream(filepath, in_stream.binary | in_stream.in);
	std::cout << "file_sender: read file: " << filepath << "\n";
	if (!in_stream.is_open()) {
		std::cerr << "file_sender: failed to open file:" << filepath << "\n";
		return EXIT_FAILURE;
	}
	while (!in_stream.eof()) {
		in_stream.read(chunk_buffer.data(), chunk_buffer.size());
		chunk_size = in_stream.gcount();
		buffer.insert(buffer.end(), chunk_buffer.begin(), chunk_buffer.begin() + chunk_size);
	}

	std::cout << "file_sender: send file\n";

	/*
	Protocol:
	<name size: std::size_t> <name> <file size: std::size_t> <file> 
	*/

	AnonymousSocket sock;
	try {
		sock = TransportInterface::requestConnect(argv[1], receiver_mip, receiver_port);
	}
	catch (TransportInterface::PortInUseException & e) {
		std::cerr << "file_sender: no port available\n";
		return EXIT_FAILURE;
	}
	catch (TransportInterface::TimeoutException & e) {
		std::cerr << "file_sender: connect timeout\n";
		return EXIT_FAILURE;
	}

	std::cout << "file_sender: buffer.size(): " << buffer.size() << "\n";

	std::size_t total_bytes_sendt = 0;
	std::string filename(extractPathLeaf(filepath));
	std::size_t filename_size = filename.size();
	total_bytes_sendt += sock.writeGeneric(filename_size);
	total_bytes_sendt += sock.write(filename.data(), filename_size);

	std::size_t buffer_size = buffer.size();
	total_bytes_sendt += sock.writeGeneric(buffer_size);
	total_bytes_sendt += sock.write(buffer.data(), buffer_size);
	
	
	sock.closeResources();

	std::cout << "file_sender: transmission done\n";
	std::cout << "file_sender: total_bytes_sendt: " << total_bytes_sendt << "\n";

	std::cout << "file_sender: terminating\n";

	return 0;
}
