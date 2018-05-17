//EventPoll.cpp
//Author: Sivert Andresen Cubedo

#include "../include/EventPoll.hpp"

EventPoll::EventPoll()
{
	m_fd = epoll_create(100);
	if (m_fd == -1) {
		throw LinuxException::Error("epoll_create()");
	}

}

EventPoll::~EventPoll()
{
	close(m_fd);
}

bool EventPoll::wait(std::vector<int> & vec)
{
	if (vec.size() != max_events) {
		vec.resize(max_events);
	}
	int e_size = epoll_wait(m_fd, vec.data(), vec.size(), -1);
	if (e_size > 0) {
		if (vec.size() != e_size) {
			vec.resize(e_size);
		}
		return true;
	}
	else if (e_size == 0) {
		//if this them timeout
		return false;
	}
	else {
		return false;
	}
}

void EventPoll::add(int fd)
{
	struct epoll_event ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = fd;
	if (epoll_ctl(m_fd, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1) {
		throw LinuxException::Error("epoll_ctl()");
	}
}

void EventPoll::remove(int fd)
{
	struct epoll_event ev;
	if (epoll_ctl(m_fd, EPOLL_CTL_DEL, fd, &ev)) {
		throw LinuxException::Error("epoll_ctl()");
	}
}
