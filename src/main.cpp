//main.cpp
//Author: Sivert Andresen Cubedo

#include <iostream>
#include <string>

//local
#include "../include/CrossIPC.hpp"

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



	std::cout << "Writing in a\n";

	a.close();
	b.close();



	return 0;
}
