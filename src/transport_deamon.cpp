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
#include "../include/TransportInterface.hpp"
#include "../include/ClientHandler.hpp"

/*
Globals
*/
ChildProcess mip_deamon;
EventPoll epoll;
AnonymousSocketPacket transport_sock;
NamedSocket application_sock;
int application_timeout;
std::vector<std::unique_ptr<ClientHandler>> client_vector;
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
	client_vector
*/
bool isPortFree(Port port)
{
	if (std::find_if(
		client_vector.begin(),
		client_vector.end(),
		[&](std::unique_ptr<ClientHandler> & ptr) { return ptr->getSourcePort() == port; })
		!= client_vector.end()
	) {
		return false;
	}
	else {
		return true;
	}
}

/*
Get a free port, if no port is free return 0 (invalid port).
Parameters:
Return:
	port
Global:
	isPortFree()
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
		//find ClientHandler to pass frame to
		Port frame_dest_port = frame.getDest();
		std::vector<std::unique_ptr<ClientHandler>>::iterator client_it;
		if (client_it = std::find_if(
			client_vector.begin(),
			client_vector.end(),
			[&](std::unique_ptr<ClientHandler> & ptr) {return frame_dest_port == ptr->getSourcePort(); }),
			client_it != client_vector.end())
		{
			//if this then frame is to client_it
			(*client_it)->receiveFrame(source, frame);
			if ((*client_it)->getStage() == ClientHandler::stage_failure) {
				client_vector.erase(client_it);

				std::cout << "transport_deamon: ClientHandler::stage_failure\n";
			
			}
		}
		else {
			std::cout << "transport_deamon: nothing at port: " << (int)frame_dest_port << "\n";
		}
	}
	catch (LinuxException::WouldBlockException & e) {
		//if wouldblock then do nothing
		return;
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
	client_vector
*/
void receiveApplicationSock()
{
	AnonymousSocket sock = application_sock.acceptConnection();
	sock.enableNonBlock();
	TimerWrapper timer;
	epoll.add(sock.getFd(), EPOLLET | EPOLLIN);
	epoll.addFriend(timer);
	client_vector.emplace_back(new ClientHandler(sock, timer, application_timeout, frame_out_queue, isPortFree, getFreePort));
}

/*
Handle ClientHandler sock.
Parameters:
	client_it		ref to iterator
Return:
	void
Global:
	client_Vector
	frame_out_queue
*/
void handleClientHandlerSock(std::vector<std::unique_ptr<ClientHandler>>::iterator & client_it)
{
	(*client_it)->handleSock();
	if ((*client_it)->getStage() == ClientHandler::stage_failure) {
		client_vector.erase(client_it);

		std::cout << "transport_deamon: ClientHandler::stage_failure\n";

	}
}

/*
Handle ClientHandler timer.
Parameters:
	client_it		ref to iterator
Return:
	void
Global:
	client_Vector
	frame_out_queue
*/
void handleClientHandlerTimer(std::vector<std::unique_ptr<ClientHandler>>::iterator & client_it)
{
	(*client_it)->handleTimer();
	if ((*client_it)->getStage() == ClientHandler::stage_failure) {
		client_vector.erase(client_it);

		std::cout << "transport_deamon: ClientHandler::stage_failure\n";
	}
}

/*
Signal function.
*/
void sigintHandler(int signum)
{
	epoll.closeResources();
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
	application_timeout = std::atoi(argv[2]);

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
				std::vector<std::unique_ptr<ClientHandler>>::iterator client_it;
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
				else if (client_it = std::find_if(
					client_vector.begin(),
					client_vector.end(),
					[&](std::unique_ptr<ClientHandler> & ptr) { return ptr->getSock().getFd() == in_fd; }),
					client_it != client_vector.end())
				{
					std::cout << "transport_deamon: client_it sock\n";
					handleClientHandlerSock(client_it);
				}
				else if (client_it = std::find_if(
					client_vector.begin(),
					client_vector.end(),
					[&](std::unique_ptr<ClientHandler> & ptr) {return ptr->getTimer().getFd() == in_fd; }),
					client_it != client_vector.end())
				{
					std::cout << "transport_deamon: client_it timer\n";
					handleClientHandlerTimer(client_it);
				}
				else {
					//assert(false);
				}
				//try to send
				sendTransportSock();
			}
		}
		catch (LinuxException::InterruptedException & e) {
			//if this then interrupted
			std::cout << "transport_deamon: epoll interrupted\n";
		}
	}

	transport_sock.closeResources();
	application_sock.closeResources();

	std::cout << "transport_deamon: joining MIP_deamon\n";

	mip_deamon.join();

	std::cout << "transport_deamon: terminating\n";

	return 0;
}



