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

enum update_sock_option
{
	LOCAL_MIP = 1,
	ARP_DISCOVERY = 2,
	ARP_LOSTCONNECTION = 3,
	ADVERTISEMENT = 4
};

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
			update_sock.write(reinterpret_cast<char*>(&dest_mip), sizeof(dest_mip));
			update_sock.write(reinterpret_cast<char*>(&buf_size), sizeof(buf_size));
			update_sock.write(buf.data(), buf_size);
		}
	}

	std::cout << distance_vector_table.toString() << "\n";
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
	std::uint8_t option;
	MIPAddress mip;
	std::size_t recv_buf_size;
	static std::vector<char> recv_buf;
	static DistanceVectorTable advert_table;

	std::vector<MIPAddress>::iterator neighbour_it;
	std::vector<DistanceVectorTable::Column>::iterator distance_it;
	
	update_sock.read(reinterpret_cast<char*>(&option), sizeof(option));
	switch (option)
	{
	case update_sock_option::LOCAL_MIP:
		update_sock.read(reinterpret_cast<char*>(&mip), sizeof(mip));
		distance_vector_table.add(mip, mip, 0);
		break;
	case update_sock_option::ARP_DISCOVERY:
		update_sock.read(reinterpret_cast<char*>(&mip), sizeof(mip));
		neighbour_it = std::find(neighbour_mip_vec.begin(), neighbour_mip_vec.end(), mip);
		if (neighbour_it == neighbour_mip_vec.end()) {
			neighbour_mip_vec.push_back(mip);
			distance_vector_table.addArpDiscovery(mip);
			sendAdvertisment(mip);
		}
		break;
	case update_sock_option::ARP_LOSTCONNECTION:
		update_sock.read(reinterpret_cast<char*>(&mip), sizeof(mip));
		neighbour_it = std::find(neighbour_mip_vec.begin(), neighbour_mip_vec.end(), mip);
		if (neighbour_it == neighbour_mip_vec.end()) {
			neighbour_mip_vec.erase(neighbour_it);
			distance_vector_table.setViaInfinity(mip);
			sendAdvertisment(mip);
		}
		break;
	case update_sock_option::ADVERTISEMENT:
		//receive:
		//	from
		//	ad size
		//	ad
		update_sock.read(reinterpret_cast<char*>(&mip), sizeof(mip));
		update_sock.read(reinterpret_cast<char*>(&recv_buf_size), sizeof(recv_buf_size));
		if (recv_buf.size() != recv_buf_size) {
			recv_buf.resize(recv_buf_size);
		}
		update_sock.read(recv_buf.data(), recv_buf.size());
		advert_table.unpackAdvertisment(recv_buf);
		if (distance_vector_table.update(mip, advert_table)) {
			sendAdvertisment(mip);
		}
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

