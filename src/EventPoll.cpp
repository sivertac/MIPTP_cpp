//EventPoll.cpp
//Author: Sivert Andresen Cubedo

#include "../include/EventPoll.hpp"

EventPoll::EventPoll() : 
	m_closed(true)
{
}

EventPoll::EventPoll(int init)
{
	m_fd = epoll_create(init);
	if (m_fd == -1) {
		throw LinuxException::Error("epoll_create()");
	}
	m_closed = false;
}

std::vector<struct epoll_event> & EventPoll::wait(std::size_t max_event, int timeout)
{
	if (m_event_vector.size() < max_event) {
		m_event_vector.resize(max_event);
	}
	int e_size = epoll_wait(m_fd, m_event_vector.data(), static_cast<int>(max_event), timeout);
	if (e_size > 0) {
		m_event_vector.resize(e_size);
		return m_event_vector;
	}
	else if (e_size == 0) {
		//if this then timeout
		throw LinuxException::TimeoutException();
	}
	else {
		//if this then errno stuff
		if (errno == EINTR) {
			throw LinuxException::InterruptedException();
		}
		else {
			m_closed = true;
			throw LinuxException::Error("epoll_wait()");
		}
	}
}

std::vector<struct epoll_event> & EventPoll::wait(std::size_t max_event)
{
	return wait(max_event, -1);
}

void EventPoll::closeResources()
{
	m_closed = true;
	close(m_fd);
}

void EventPoll::add(int fd)
{
	add(fd, EPOLLIN);
}

void EventPoll::add(int fd, int flags)
{
	struct epoll_event ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.events = flags;
	ev.data.fd = fd;
	add(fd, ev);
}

void EventPoll::add(int fd, epoll_event & ev)
{
	if (epoll_ctl(m_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
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

bool EventPoll::isClosed()
{
	return m_closed;
}
