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
	
	CrossIPC::AnonymousSocketPair pair;

	try {
		pair = CrossIPC::createAnonymousSocketPair();
	}
	catch (CrossIPC::ErrorPipeException & e) {
		std::cerr << e.what() << "\n";
		return 0;
	}
	CrossIPC::AnonymousSocket & a = pair.first;
	CrossIPC::AnonymousSocket & b = pair.second;

	std::cout << a.toString() << "\n";
	
	CrossIPC::AnonymousSocket c(a.toString());

	std::cout << c.toString() << "\n";


	

	a.close();
	b.close();
	

	if (argc == 1) {
		std::cout << "Hello from Parent\n";
		std::string program = "MIPTP_cpp_win.exe";
		std::vector<std::string> args;
		args.push_back("hello1");
		args.push_back("hello2");

		std::cout << "Spawning Child\n";
		auto child = CrossForkExec::forkExec(program, args);
		std::cout << "Joining for Child\n";
		child.join();
		std::cout << "Child has returned\n";
		child.close();
		return 0;
	}
	else {
		std::cout << "Hello from Child\n";
		
		Sleep(2000);

		std::cout << "Child exit\n";

		return 0;
	}
}
