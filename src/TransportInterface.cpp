//TransportInterface.cpp
//Author: Sivert Andresen Cubedo

#include "../include/TransportInterface.hpp"

AnonymousSocket TransportInterface::requestListen(std::string path, Port port)
{
	AnonymousSocket sock = NamedSocket::connectToNamedSocket(path);
	
	std::uint8_t request = request_listen;
	sock.writeGeneric(request);
	sock.writeGeneric(port);
	
	std::uint8_t reply;
	sock.readGeneric(reply);

	if (reply == reply_success) {
		return sock;
	}
	else if (reply == reply_timeout) {
		throw TimeoutException();
	}
	else if (reply == reply_usedport) {
		throw PortInUseException();
	}
	assert(false);
}

AnonymousSocket TransportInterface::requestConnect(std::string path, MIPAddress address, Port port)
{
	AnonymousSocket sock = NamedSocket::connectToNamedSocket(path);

	std::uint8_t request = request_connect;
	sock.writeGeneric(request);
	sock.writeGeneric(address);
	sock.writeGeneric(port);

	std::uint8_t reply;
	sock.readGeneric(reply);

	if (reply == reply_success) {
		return sock;
	}
	else if (reply == reply_timeout) {
		throw TimeoutException();
	}
	else if (reply == reply_usedport) {
		throw PortInUseException();
	}
	assert(false);
}
