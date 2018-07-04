//file_generate.cpp
//Author: Sivert Andresen Cubedo

#include <iostream>
#include <fstream>
#include <utility>
#include <string>
#include <random>

const char* help_string = "Args: ./file_generate <filename> <size>";
/*
Args: ./file_generate <filename> <size>
*/
int main(int argc, char** argv)
{
	if (argc != 3) {
		std::cout << "Wrong args\n";
		std::cout << help_string << "\n";
		return EXIT_FAILURE;
	}
	
	std::string filename(argv[1]);
	std::size_t file_size = std::atoi(argv[2]);
	
	std::fstream file_stream(filename, file_stream.binary | file_stream.out | file_stream.trunc);
	if (!file_stream.is_open()) {
		std::cerr << "file_generate: failed to open/create: " << filename << "\n";
		return EXIT_FAILURE;
	}

	//init random generator
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<std::uint8_t> dist;

	for (std::size_t i = 0; i < file_size; ++i) {
		file_stream.put(dist(mt));
	}

	std::cout << filename << " generated\n";

	return 0;
}
