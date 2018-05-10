//main.cpp
//Author: Sivert Andresen Cubedo

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

//local
#include "../include/CrossIPC.hpp"
#include "../include/CrossForkExec.hpp"

int main(int argc, char** argv)
{	
	auto pair = CrossIPC::createAnonymousSocketPair();
	CrossIPC::AnonymousSocket & child_sock = pair.first;
	
	std::cout << "Parent spawning child\n";
	auto child = CrossForkExec::forkExec("test_child_win.exe", { pair.second.toString() });

	Sleep(150);
	std::cout << "Parent reading from child\n";
	std::string str = child_sock.readString();
	std::cout << str << "\n";
	std::cout << "String len: " << str.length() << "\n";

	Sleep(100);
	std::cout << "Parent writing to child\n";
	str = "Hello from parenjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj76598795688709657687659760987986jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjt";
	child_sock.writeString(str);
	std::cout << "String len: " << str.length() << "\n";

	std::cout << "Parent joining child\n";
	child.join();
	
	std::cout << "Parent done\n";





	return 0;
}
