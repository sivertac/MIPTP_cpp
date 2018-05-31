//Author: Sivert Andresen Cubedo
//EventPoll.hpp

#pragma once
#ifndef EventPoll_HEADER
#define EventPoll_HEADER

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <array>
#include <exception>
#include <stdexcept>
#include <cassert>
#include <type_traits>

#include <unistd.h>
#include <sys/epoll.h>

#include "LinuxException.hpp"

class EventPoll
{
public:
	enum WaitState {
		Error,
		Timeout,
		Okay
	};

	/*
	Construtor.
	Not a valid epoll.
	*/
	EventPoll();
	
	/*
	Constructor.
	Valid epoll.
	Parameters:
		epoll_init		value to init epoll ( see epoll_create(2) )
	*/
	EventPoll(int init);

	/*
	Wait until one or more fd is ready (will block).
	Parameters:
		max_event	max number of events to pull
		timeout		epoll timeout parameter
	Return:
		WaitState enum
	*/
	WaitState wait(std::size_t max_event, int timeout);
	WaitState wait(std::size_t max_event);

	/*
	Close epoll (makes m_fd invalid).
	Parameters:
	Return:
		void
	*/
	void closeResources();

	/*
	Add file descriptor to epoll.
	Parameters:
		fd		file descriptor to add
	Return:
		void
	*/
	void add(int fd);

	/*
	Add friend to EventPoll.
	Parameters:
		friend		ref to friend that has a m_fd member
	Returns:
		void
	*/
	template <class T>
	void addFriend(T & obj)
	{
		this->add(obj.getFd());
	}
	

	/*
	Remove file descriptor from epoll.
	Parameters:
		fd		file descriptor to remove
	Return:
		void
	*/
	void remove(int fd);

	/*
	Check if epoll is closed.
	Parameters:
	Return:
		if true close, else valid
	*/
	bool isClosed();

	/*
	Remove friend from EventPoll.
	Parameters:
		friend	ref to friend that has a m_fd member
	return:
		void
	*/
	template <class T>
	void removeFriend(T & obj)
	{
		this->remove(obj.getFd());
	}

	/*
	For holding events from epoll_wait() (capacity must be >= max_event).
	*/
	std::vector<struct epoll_event> m_event_vector;
private:
	int m_fd;
	bool m_closed;
};

#endif
