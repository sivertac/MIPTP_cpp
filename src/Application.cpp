//Application.cpp
//Author: Sivert Andresen Cubedo

#include "../include/Application.hpp"

Application::Application(Port port, AnonymousSocket & sock)
{
}

Port Application::getPort()
{
	return m_port;
}

AnonymousSocket & Application::getSock()
{
	return m_sock;
}

ApplicationClient::ApplicationClient(Port port, AnonymousSocket & sock) :
	Application(port, sock)
{
}
