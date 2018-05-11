//main.cpp
//Author: Sivert Andresen Cubedo

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

//local
#include "../include/RawSock.hpp"			//Uses winsock2.h must be before stuff that uses Windows.h //NEED TO LINK Iphlpapi.lib
#include "../include/CrossIPC.hpp"
#include "../include/CrossForkExec.hpp"

int main(int argc, char** argv)
{	
	
	for (auto & name : RawSock::getInterfaceNames()) {
		std::cout << std::string(name.begin(), name.end()) << "\n";
	}




	return 0;
}
