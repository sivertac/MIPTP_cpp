//file_compare.cpp
//Author: Sivert Andresen Cubedo

#include <iostream>
#include <fstream>
#include <utility>
#include <string>

const char* help_string = "Args: ./file_compare <file 1> <file 2>";
/*
Args: ./file_compare <file1> <file2>
*/
int main(int argc, char** argv)
{
	if (argc != 3) {
		std::cout << "Wrong args\n";
		std::cout << help_string << "\n";
		return EXIT_FAILURE;
	}
	
	std::string file1_path(argv[1]);
	std::string file2_path(argv[2]);

	//open file1
	std::fstream file1_stream(file1_path, file1_stream.binary | file1_stream.in);
	if (!file1_stream.is_open()) {
		std::cerr << "file_compare: failed to open file1: " << file1_path << "\n";
		return EXIT_FAILURE;
	}

	//open file2
	std::fstream file2_stream(file2_path, file2_stream.binary | file2_stream.in);
	if (!file2_stream.is_open()) {
		std::cerr << "file_compare: failed to open file2: " << file2_path << "\n";
		return EXIT_FAILURE;
	}

	//compare
	bool equal = true;
	while (!file1_stream.eof() && !file2_stream.eof()) {
		if (file1_stream.get() != file2_stream.get()) {
			equal = false;
			break;
		}
	}
	if (file1_stream.eof() != file2_stream.eof()) {
		equal = false;
	}

	//print
	std::cout << std::boolalpha;
	std::cout << equal << "\n";;

	return 0;
}
