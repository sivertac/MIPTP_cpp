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
#include <functional>

//LINUX
#include <signal.h>

//Local
#include "../include/LinuxException.hpp"
#include "../include/AddressTypes.hpp"
#include "../include/CrossIPC.hpp"
#include "../include/CrossForkExec.hpp"
#include "../include/EventPoll.hpp"
#include "../include/Application.hpp"
#include "../include/TransportInterface.hpp"

/*
Globals
*/
ChildProcess mip_deamon;
EventPoll epoll;
AnonymousSocketPacket transport_sock;
NamedSocket application_sock;
std::vector<std::unique_ptr<ApplicationServer>> server_vector;
std::vector<std::unique_ptr<ApplicationClient>> client_vector;
std::vector<AnonymousSocket> pending_request_vector;
std::queue<std::pair<MIPAddress, MIPTPFrame>> frame_out_queue;

/*
Application request:
	<request type : uint8_t> <dest mip : MIPAddress> <port : Port>
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
	dest		destination of frame
	frame		MIPTPFrame to send
Return:
	void
Global:
	transport_sock
	frame_out_queue

*/
void sendTransportSock()
{
	std::cout << "transport_deamon: sendTransportSock()\n";
	//try to send frames until wouldblock
	while (!frame_out_queue.empty()) {
		try {
			MIPAddress dest = frame_out_queue.front().first;
			MIPTPFrame & frame = frame_out_queue.front().second;
			std::size_t msg_size = frame.getSize();
			AnonymousSocketPacket::IovecWrapper<3> iov;
			iov.setIndex(0, dest);
			iov.setIndex(1, msg_size);
			iov.setIndex(2, frame.getData(), msg_size);
			transport_sock.sendiovec(iov);
		}
		catch (LinuxException::WouldBlockException & e) {
			return;
		}
		frame_out_queue.pop();
	}
}

/*
Receive on transport_sock.
Parameters:
Return:
	void
Global:
	transport_sock
	server_vector
	client_vector
	frame_out_queue
*/
void receiveTransportSock()
{
	std::cout << "transport_deamon: Receive transport_sock" << "\n";
	try {
		static MIPTPFrame frame;
		if (frame.getSize() != MIPTPFrame::FRAME_MAX_SIZE) {
			frame.setSize(MIPTPFrame::FRAME_MAX_SIZE);
		}
		MIPAddress source;
		std::size_t msg_size;
		AnonymousSocketPacket::IovecWrapper<3> iov;
		iov.setIndex(0, source);
		iov.setIndex(1, msg_size);
		iov.setIndex(2, frame.getData(), frame.getSize());
		transport_sock.recviovec(iov);
		//resize frame to msg_size
		frame.setSize(msg_size);
		//find Application to pass frame to
		Port frame_dest_port = frame.getDest();
		std::vector<std::unique_ptr<ApplicationServer>>::iterator server_it;
		std::vector<std::unique_ptr<ApplicationClient>>::iterator client_it;
		if (server_it = std::find_if(
			server_vector.begin(),
			server_vector.end(),
			[&](std::unique_ptr<ApplicationServer> & ptr) {return frame_dest_port == ptr->getServerPort(); }),
			server_it != server_vector.end())
		{
			//if this then frame is to server_it
			(*server_it)->receiveFrame(frame, source);
			if ((*server_it)->getStage() == ApplicationServer::stage_failure) {
				server_vector.erase(server_it);
			}
		}
		else if (client_it = std::find_if(
			client_vector.begin(),
			client_vector.end(),
			[&](std::unique_ptr<ApplicationClient> & ptr) {return frame_dest_port == ptr->getClientPort(); }),
			client_it != client_vector.end())
		{
			//if this then frame is to client_it
			(*client_it)->receiveFrame(frame, source);
			if ((*client_it)->getStage() == ApplicationClient::stage_failure) {
				client_vector.erase(client_it);
			}
		}
	}
	catch (LinuxException::WouldBlockException & e) {
		//if wouldblock then do nothing
	}
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
	epoll.add(sock.getFd(), EPOLLET | EPOLLIN);
	pending_request_vector.push_back(sock);
}

/*
Handle ApplicationServer sock.
Parameters:
	server		ref to ApplicationServer
Return:
	void
Global:
	server_vector
	frame_out_vec
*/
void handleApplicationServerSock(std::vector<std::unique_ptr<ApplicationServer>>::iterator & server_it)
{
	(*server_it)->handleSock();
	if ((*server_it)->getStage() == ApplicationServer::stage_failure) {
		server_vector.erase(server_it);
		
		std::cout << "transport_deamon: ApplicationServer::stage_failure\n";
	}
}

/*
Handle ApplicationClient sock.
Parameters:
	client		ref to ApplicationClient
Return:
	void
Global:
	client_Vector
	frame_out_vec
*/
void handleApplicationClientSock(std::vector<std::unique_ptr<ApplicationClient>>::iterator & client_it)
{
	(*client_it)->handleSock();
	if ((*client_it)->getStage() == ApplicationClient::stage_failure) {
		client_vector.erase(client_it);

		std::cout << "transport_deamon: ApplicationClient::stage_failure\n";
	}
}

/*
Handle pending application request.
Parameters:
	sock		ref to requesting sock
Return:
	void
Global:
	epoll
	pending_request_vector
	server_vector
	client_vector
	frame_out_queue
*/
void handlePendingRequest(std::vector<AnonymousSocket>::iterator & pending_it)
{
	//<request type : uint8_t>
	AnonymousSocket & sock = *pending_it;
	std::uint8_t request;
	pending_it->readGeneric<std::uint8_t>(request);
	if (request ==  TransportInterface::ApplicationRequest::request_listen) {
		TimerWrapper timer;
		epoll.addFriend<TimerWrapper>(timer);
		server_vector.emplace_back(new ApplicationServer(sock, timer, frame_out_queue, isPortFree, getFreePort));
	}
	else if (request == TransportInterface::ApplicationRequest::request_connect) {
		TimerWrapper timer;
		epoll.addFriend<TimerWrapper>(timer);
		client_vector.emplace_back(new ApplicationClient(sock, timer, frame_out_queue, isPortFree, getFreePort));
	}
	else {
		std::cout << "transport_deamon: Invalid application request\n";
		bool reply = false;
		epoll.removeFriend<AnonymousSocket>(sock);
		sock.writeGeneric<bool>(reply);
		sock.closeResources();
	}
	pending_request_vector.erase(pending_it);
}

/*
Signal function.
*/
void sigintHandler(int signum)
{
	
}

const char* help_string = "./transport_deamon <named socket name> <timeout> [mip addresses]";
/*
args: ./transport_deamon <named socket name> <timeout> [mip addresses]
*/
int main(int argc, char** argv)
{
	//parse args
	if (argc < 4) {
		std::cerr << "Too few args\n";
		std::cout << help_string << "\n";
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
		auto pair = AnonymousSocketPacket::createPair();
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

	//configure sockets
	transport_sock.enableNonBlock();
	epoll.add(transport_sock.getFd(), EPOLLIN | EPOLLET);
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
					std::cout << "transport_deamon: transport_sock\n";
					//try to receive
					receiveTransportSock();
					//try to send
					sendTransportSock();
				}
				else if (in_fd == application_sock.getFd()) {
					std::cout << "transport_deamon: application_sock\n";
					receiveApplicationSock();
				}
				else if (pending_it = std::find_if(
					pending_request_vector.begin(),
					pending_request_vector.end(),
					[&](AnonymousSocket & s) { return s.getFd() == in_fd; }),
					pending_it != pending_request_vector.end()) 
				{
					std::cout << "transport_deamon: pending_it\n";
					handlePendingRequest(pending_it);
				}
				else if (server_it = std::find_if(
					server_vector.begin(),
					server_vector.end(),
					[&](std::unique_ptr<ApplicationServer> & ptr) { return ptr->getSock().getFd() == in_fd; }),
					server_it != server_vector.end())
				{
					std::cout << "transport_deamon: server_it\n";
					handleApplicationServerSock(server_it);
				}
				else if (client_it = std::find_if(
					client_vector.begin(),
					client_vector.end(),
					[&](std::unique_ptr<ApplicationClient> & ptr) { return ptr->getSock().getFd() == in_fd; }),
					client_it != client_vector.end())
				{
					std::cout << "transport_deamon: client_it\n";
					handleApplicationClientSock(client_it);
				}
				else {
					assert(false);
				}
				//try to send
				sendTransportSock();
			}
		}
		catch (LinuxException::InterruptedException & e) {
			//if this then interrupted
			std::cout << "transport_deamon: epoll interrupted\n";
			epoll.closeResources();
		}
	}

	transport_sock.closeResources();
	application_sock.closeResources();

	std::cout << "transport_deamon: joining MIP_deamon\n";

	mip_deamon.join();

	std::cout << "transport_deamon: terminating\n";

	return 0;
}



