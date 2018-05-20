//EventPoll.cpp
//Author: Sivert Andresen Cubedo

#include "../include/EventPoll.hpp"

EventPoll::EventPoll() :
	m_event_vector(max_event)
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

EventPoll::WaitState EventPoll::wait(int timeout)
{
	if (m_event_vector.size() < max_event) {
		m_event_vector.resize(max_event);
	}
	int e_size = epoll_wait(m_fd, m_event_vector.data(), max_event, timeout);
	if (e_size > 0) {
		m_event_vector.resize(e_size);
		return Okay;
	}
	else if (e_size == 0) {
		//if this them timeout
		return Timeout;
	}
	else {
		return Error;
	}
}

EventPoll::WaitState EventPoll::wait()
{
	return wait(-1);
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
