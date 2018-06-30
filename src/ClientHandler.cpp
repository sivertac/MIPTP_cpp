//ClientHandler.cpp
//Author: Sivert Andresen Cubedo

#include "../include/ClientHandler.hpp"

ClientHandler::ClientHandler(AnonymousSocket & sock, TimerWrapper & timer, std::queue<std::pair<MIPAddress, MIPTPFrame>>& out_queue, std::function<bool(Port)> is_port_free, std::function<Port()> get_free_port)
{

}
	