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
#include <thread>
#include <queue>
#include <condition_variable>


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
//main thread is receive thread
std::thread update_thread;
std::thread lookup_thread;
std::mutex update_mutex;
std::mutex lookup_mutex;
std::condition_variable update_blocker;
std::condition_variable lookup_blocker;
std::queue<MIPAddress> update_queue;					//contains order to exclude ads sendt
std::queue<MIPAddress> lookup_queue;					//contains request mip
bool running = true;

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
}

/*
Receive on update_sock.
Parameters:
Return:
	void
Global:
	update_sock
	distance_vector_table
	neighbour_mip_vec
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
		if (neighbour_it != neighbour_mip_vec.end()) {
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
	std::cout << distance_vector_table.toString() << "\n";
}

/*
Receive on lookup_sock.
Parameters:
Return:
	void
Global:
	lookup_sock
	distance_vector_table
*/
void receiveLookupSock()
{
	//receive:
	//	mip
	MIPAddress mip;
	lookup_sock.read(reinterpret_cast<char*>(&mip), sizeof(mip));
	{
		std::lock_guard<std::mutex> lock(lookup_mutex);
		lookup_queue.push(mip);
	}
	lookup_blocker.notify_one();
}

/*
Handler for update_thread.
*/
void update_thread_handler()
{

	{
		std::unique_lock<std::mutex> lock(update_mutex);
		update_blocker.wait(lock, [] { return !update_queue.empty(); });
	}


}

/*
Handler for lookup_thread.
*/
void lookup_thread_handler()
{
	std::cout << "lookup_thread start\n";
	while (true) {
		MIPAddress mip;
		{
			std::unique_lock<std::mutex> lock(lookup_mutex);
			lookup_blocker.wait(lock, [] { return !lookup_queue.empty() || !running; });



			mip = lookup_queue.front();
			lookup_queue.pop();
		}
		auto dis_it = distance_vector_table.findTo(mip);
		if (dis_it != distance_vector_table.m_data.end()) {
			//reply:
			//	search success
			//	next hop
			const bool succ = true;
			lookup_sock.write(reinterpret_cast<const char*>(&succ), sizeof(succ));
			lookup_sock.write(reinterpret_cast<char*>(&dis_it->via), sizeof(MIPAddress));
		}
		else {
			//reply:
			//	search failure
			const bool succ = false;
			lookup_sock.write(reinterpret_cast<const char*>(&succ), sizeof(succ));
		}
	}
	std::cout << "lookup_thread exit\n";
}

/*
Signal function.
*/
void sigintHandler(int signum)
{
	//transport_deamon will handle this
}

/*
args: ./routing_deamon <update sock> <lookup sock>
*/
int main(int argc, char** argv)
{
	//parse args
	if (argc < 3) {
		std::cerr << "Too few args\n";
		return EXIT_FAILURE;
	}
	
	//signal
	struct sigaction sa;
	sa.sa_handler = &sigintHandler;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		throw LinuxException::Error("sigaction()");
	}

	//wrap sockets
	update_sock = AnonymousSocket(argv[1]);
	lookup_sock = AnonymousSocket(argv[2]);

	//start threads
	update_thread = std::thread(update_thread_handler);
	lookup_thread = std::thread(lookup_thread_handler);
	
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
				receiveLookupSock();
			}
		}
	}

	//cleanup
	if (update_thread.joinable()) {
		update_thread.join();
	}
	if (lookup_thread.joinable()) {
		lookup_thread.join();
	}

	std::cout << "routing_deamon is terminating\n";
	return 0;
}

