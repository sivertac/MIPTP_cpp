//test_child.cpp
//Author: Sivert Andresen Cubedo

#include <iostream>
#include <string>

#include "../include/CrossIPC.hpp"

//args: <program_name> <AnonymousSocket string>
int main(int argc, char** argv)
{
	CrossIPC::AnonymousSocket parent_sock(argv[1]);

	Sleep(100);
	std::cout << "Child writing to parent\n";
	std::string str = "Hello from child";
	parent_sock.writeString(str);
	std::cout << "String len: " << str.length() << "\n";

	Sleep(200);
	std::cout << "Child reading from parent\n";
	str = parent_sock.readString();
	std::cout << str << "\n";
	std::cout << "String len: " << str.length() << "\n";

	Sleep(100);
	std::cout << "Child done\n";
	parent_sock.close();
	
	return 0;
}


