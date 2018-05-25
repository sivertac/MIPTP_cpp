//routing_deamon.cpp
//Author: Sivert Andresen Cubedo

//C++
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <stdexcept>

//LINUX
#include <signal.h>

#include "../include/AddressTypes.hpp"
#include "../include/CrossIPC.hpp"
#include "../include/EventPoll.hpp"
#include "../include/DistanceVectorTable.hpp"
#include "../include/LinuxException.hpp"

/*
Globals
*/
AnonymousSocket update_sock;
AnonymousSocket lookup_sock;
std::vector<MIPAddress> neighbour_mip_vec;
DistanceVectorTable distance_vector_table;

/*
Send advertisment to all direct neighbours.
Parameters:
	exclude		exclude this address
Return:
	void
Global:
	update_sock
	neighbour_mip_vec
	distance_vector_table
*/
void sendAdvertisment(MIPAddress exclude)
{
	//send:
	//	dest
	//	ad size
	//	ad
	static std::vector<char> buf;
	std::size_t buf_size;
	for (MIPAddress dest_mip : neighbour_mip_vec) {
		if (dest_mip != exclude) {
			distance_vector_table.packAdvertisment(buf, dest_mip);
			buf_size = buf.size();
			update_sock.write(reinterpret_cast<char*>(dest_mip), sizeof(dest_mip));
			update_sock.write(reinterpret_cast<char*>(buf_size), sizeof(buf_size));
			update_sock.write(buf.data(), buf_size);
		}
	}
}

/*
Receive on update_sock.
Parameters:
Return:
	void
Global:
	update_sock
	distance_vector_table
*/
void receiveUpdateSock()
{
	enum opt {
		LOCAL_MIP = 0,
		ARP_DISCOVERY = 1,
		ARP_LOSTCONNECTION = 2,
		ADVERTISEMENT = 3
	};
	static DistanceVectorTable advert_table;
	std::uint8_t option;
	MIPAddress mip;
	std::vector<MIPAddress>::iterator neighbour_it;
	std::vector<DistanceVectorTable::Column>::iterator distance_it;
	update_sock.read(reinterpret_cast<char*>(&option), sizeof(option));
	switch (option)
	{
	case opt::LOCAL_MIP:
		update_sock.read(reinterpret_cast<char*>(&mip), sizeof(mip));
		distance_vector_table.add(mip, mip, 0);
		break;
	case opt::ARP_DISCOVERY:
		update_sock.read(reinterpret_cast<char*>(&mip), sizeof(mip));
		neighbour_it = std::find(neighbour_mip_vec.begin(), neighbour_mip_vec.end(), mip);
		if (neighbour_it == neighbour_mip_vec.end()) {
			neighbour_mip_vec.push_back(mip);
			distance_vector_table.addArpDiscovery(mip);
			
		}
		break;
	case opt::ARP_LOSTCONNECTION:
		break;
	case opt::ADVERTISEMENT:
		break;
	default:
		throw std::runtime_error("Invalid option");
		break;
	}


}

/*
Receive on lookup_sock.
Parameters:
Return:
	void
Global:
	lookup_sock
*/
void receiveLookupSock()
{

}

/*
args: ./routinh_deamon <update sock> <lookup sock>
*/
int main(int argc, char** argv)
{
	//parse args
	if (argc < 3) {
		std::cerr << "Too few args\n";
		return EXIT_FAILURE;
	}
	
	update_sock = AnonymousSocket(argv[1]);
	lookup_sock = AnonymousSocket(argv[2]);

	
	EventPoll epoll;

	epoll.addFriend<AnonymousSocket>(update_sock);
	epoll.addFriend<AnonymousSocket>(lookup_sock);

	while (epoll.wait() == EventPoll::Okay) {
		for (auto & ev : epoll.m_event_vector) {
			int in_fd = ev.data.fd;
			
			if (in_fd == update_sock.getFd()) {
				receiveUpdateSock();
			}
			else if (in_fd == lookup_sock.getFd()) {

			}
		}
	}

	std::cout << "routing_deamon is terminating\n";

	return 0;
}

