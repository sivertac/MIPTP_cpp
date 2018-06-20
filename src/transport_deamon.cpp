//transport_deamon.cpp
//Author: Sivert Andresen Cubedo

//C++
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <memory>

//LINUX
#include <signal.h>

//Local
#include "../include/LinuxException.hpp"
#include "../include/AddressTypes.hpp"
#include "../include/CrossIPC.hpp"
#include "../include/CrossForkExec.hpp"
#include "../include/EventPoll.hpp"
#include "../include/Application.hpp"

/*
Globals
*/
ChildProcess mip_deamon;
EventPoll epoll;
AnonymousSocket transport_sock;
NamedSocket application_sock;
std::vector<std::unique_ptr<ApplicationServer>> server_vector;
std::vector<std::unique_ptr<ApplicationClient>> client_vector;
std::vector<AnonymousSocket> pending_request_vector;
std::queue<MIPTPFrame> frame_out_queue;

/*
Application request:
	<request type : uint8_t> <port : Port>
	request type :
		1 : listen
		2 : connect
	only 14 lower bits are part of port.

Application request reply:
	<reply : bool>
	reply :
		true	: granted
		false	: not granted

transport protocol:
	ApplicationServer is listening on Port p1.
	ApplicationClient sends connect request from Port p2 to ApplicationServer on p1.
	ApplicationServer replies accept if connection is accepted or not.

2 way communication is possible.
*/

/*
Check if port is free.
Parameters:
	port		port to check
Return:
	true if free, flase if used
Global:
	server_vector
	client_vector
*/
bool isPortFree(Port port)
{
	if (std::find_if(
		server_vector.begin(),
		server_vector.end(),
		[&](std::unique_ptr<ApplicationServer> & ptr) { return ptr->getServerPort() == port; }) != server_vector.end()
	) {
		return false;
	}
	if (std::find_if(
		client_vector.begin(),
		client_vector.end(),
		[&](std::unique_ptr<ApplicationClient> & ptr) { return ptr->getClientPort() == port; }) != client_vector.end()
	) {
		return false;
	}
	return true;
}

/*
Get a free port, if no port is free return 0 (invalid port).
Parameters:
Return:
	port
Global:
	server_vector
	client_vector
*/
Port getFreePort()
{
	const Port max_port = 0b11111111111111;
	Port port = 1;
	while (port < max_port) {
		if (isPortFree(port)) {
			return port;
		}
		++port;
	}
	return 0;
}

/*
Send on transport_sock.
Parameters:
	dest
	msg
Return:
	void
Global:
	transport_sock
*/
void sendTransportSock(MIPAddress dest, std::vector<char> & msg)
{
	//send
	//	dest
	//	msg size
	//	msg
	std::size_t msg_size = msg.size();
	transport_sock.write(reinterpret_cast<char*>(&dest), sizeof(dest));
	transport_sock.write(reinterpret_cast<char*>(&msg_size), sizeof(msg_size));
	transport_sock.write(reinterpret_cast<char*>(msg.data()), msg_size);
}

/*
Receive on application_sock.
Parameters:
Return:
	void
Global:
	epoll
	application_sock
	pending_request_vector
*/
void receiveApplicationSock()
{
	AnonymousSocket sock = application_sock.acceptConnection();
	epoll.addFriend<AnonymousSocket>(sock);
	pending_request_vector.push_back(sock);
}

/*
Handle pending application request.
Parameters:
	sock		ref to requesting sock
Return:
	void
Global:
	epoll
	server_vector
	client_vector
	frame_out_queue
*/
void handlePendingRequest(AnonymousSocket & sock)
{
	//<request type : uint8_t> <port : Port>
	enum ApplicationRequest
	{
		request_listen = 1,
		request_connect = 2
	};
	epoll.removeFriend<AnonymousSocket>(sock);
	std::uint8_t request;
	Port request_port;
	bool reply;
	sock.readGeneric<std::uint8_t>(request);
	sock.readGeneric<Port>(request_port);
	if (request == request_listen) {
		//if this then check if port is free
		//	if port is free create ApplicationServer object
		//	else decline request and close connection
		std::cout << "Application listen request\n";
		if (isPortFree(request_port)) {
			sock.enableNonBlock();
			server_vector.emplace_back(new ApplicationServer(request_port, sock, frame_out_queue));
			epoll.add(sock.getFd(), EPOLLET | EPOLLIN);
			reply = true;
			sock.writeGeneric<bool>(reply);
		}
		else {
			std::cout << "Port not free\n";
			reply = false;
			sock.writeGeneric<bool>(reply);
			sock.closeResources();
		}
	}
	else if (request == request_connect) {
		//if this then create ApplicationClient object and attemt handshake
		std::cout << "Application connect request\n";
		Port client_port = getFreePort();
		if (client_port != 0) {
			sock.enableNonBlock();
			client_vector.emplace_back(new ApplicationClient(client_port, request_port, sock, frame_out_queue));
			epoll.add(sock.getFd(), EPOLLET | EPOLLIN);
		}
		else {
			std::cout << "No free port\n";
			reply = false;
			sock.writeGeneric<bool>(reply);
			sock.closeResources();
		}
	}
	else {
		std::cout << "Invalid application request\n";
		reply = false;
		sock.writeGeneric<bool>(reply);
		sock.closeResources();
	}
}

/*
Receive on transport_sock.
Parameters:
Return:
	void
Global:
	transport_sock
*/
void receiveTransportSock()
{
	//receive
	//	source
	//	msg size
	//	msg
	MIPAddress source_mip;
	std::size_t msg_size;
	static std::vector<char> msg;
	transport_sock.read(reinterpret_cast<char*>(&source_mip), sizeof(source_mip));
	transport_sock.read(reinterpret_cast<char*>(&msg_size), sizeof(msg_size));
	if (msg.size() != msg_size) {
		msg.resize(msg_size);
	}
	transport_sock.read(msg.data(), msg_size);

	//test
	std::cout << std::string(msg.data()) << "\n";
}

/*
Signal function.
*/
void sigintHandler(int signum)
{
	epoll.closeResources();
}

/*
args: ./transport_deamon <socket_application> <timeout> [mip addresses]
*/
int main(int argc, char** argv)
{
	//parse args
	if (argc < 4) {
		std::cerr << "Too few args\n";
		return EXIT_FAILURE;
	}

	//signal
	struct sigaction sa;
	sa.sa_handler = &sigintHandler;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		throw LinuxException::Error("sigaction()");
	}

	//Connect MIP_deamon
	{
		auto pair = AnonymousSocket::createAnonymousSocketPair();
		std::vector<std::string> args(1);
		args[0] = pair.second.toString();
		for (int i = 3; i < argc; ++i) {
			args.push_back(argv[i]);
		}
		mip_deamon = ChildProcess::forkExec("./MIP_deamon", args);
		transport_sock = pair.first;
		pair.second.closeResources();
	}

	//socket application
	{
		std::string name(argv[1]);
		application_sock = NamedSocket(name);
	}

	//epoll
	epoll = EventPoll(100);

	epoll.addFriend<AnonymousSocket>(transport_sock);
	epoll.addFriend<NamedSocket>(application_sock);

	while (!epoll.isClosed()) {
		try {
			auto & event_vec = epoll.wait(20);
			for (auto & ev : event_vec) {
				std::vector<AnonymousSocket>::iterator pending_it;
				std::vector<std::unique_ptr<ApplicationServer>>::iterator server_it;
				std::vector<std::unique_ptr<ApplicationClient>>::iterator client_it;
				int in_fd = ev.data.fd;
				if (in_fd == transport_sock.getFd()) {
					receiveTransportSock();
				}
				else if (in_fd == application_sock.getFd()) {
					receiveApplicationSock();
				}
				else if (pending_it = std::find_if(
					pending_request_vector.begin(),
					pending_request_vector.end(),
					[&](AnonymousSocket & s) { return s.getFd() == in_fd; }),
					pending_it != pending_request_vector.end()) 
				{
					std::cout << "pending_it\n";
				}
				else if (server_it = std::find_if(
					server_vector.begin(),
					server_vector.end(),
					[&](std::unique_ptr<ApplicationServer> & ptr) { return ptr->getSock().getFd() == in_fd; }),
					server_it != server_vector.end())
				{
					std::cout << "server_it\n";
				}
				else if (client_it = std::find_if(
					client_vector.begin(),
					client_vector.end(),
					[&](std::unique_ptr<ApplicationClient> & ptr) { return ptr->getSock().getFd() == in_fd; }),
					client_it != client_vector.end())
				{
					std::cout << "client_it\n";
				}
				else {
					assert(false);
				}
			}
		}
		catch (LinuxException::InterruptedException & e) {
			//if this then interrupted
			std::cout << "transport_deamon: epoll interrupted\n";
		}
	}

	std::cout << "transport_deamon joining mip_deamon\n";

	transport_sock.closeResources();
	application_sock.closeResources();

	mip_deamon.join();

	std::cout << "transport_deamon terminating\n";

	return 0;
}



