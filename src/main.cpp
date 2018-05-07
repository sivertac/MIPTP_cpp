//main.cpp
//Author: Sivert Andresen Cubedo

#include <iostream>
#include <string>

int main(int argc, char** argv)
{

	std::cout << "Hello World!\n";

#ifdef WINDOWS
	std::cout << "WINDOWS BINARY\n";
#elif UNIX
	std::cout << "UNIX BINARY\n";
#endif





	return 0;
}
