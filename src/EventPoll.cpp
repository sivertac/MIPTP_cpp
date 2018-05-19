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

bool EventPoll::wait(std::vector<int> & fd_vec)
{
	int e_size = epoll_wait(m_fd, m_event_array.data(), m_event_array.size(), -1);
	if (e_size > 0) {
		fd_vec.resize(e_size);
		for (int i = 0; i < e_size; ++i) {
			fd_vec[i] = m_event_array[i].data.fd;
		}
		return true;
	}
	else if (e_size == 0) {
		//if this them timeout
		return false;
	}
	else {
		throw LinuxException::Error("epoll_wait()");
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
