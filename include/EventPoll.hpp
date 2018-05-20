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
	*/
	EventPoll();

	/*
	Deconstructor.
	*/
	~EventPoll();

	/*
	Wait until one or more fd is ready (will block).
	Parameters:
		timeout		epoll timeout parameter
	Return:
		WaitState enum
	*/
	WaitState wait(int timeout);
	WaitState wait();

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
		//static_assert(std::is_integral<typename T::m_fd>::value, "This member is required");
		this->add(obj.m_fd);
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
	Remove friend from EventPoll.
	Parameters:
		friend	ref to friend that has a m_fd member
	return:
		void
	*/
	template <class T>
	void removeFriend(T & obj)
	{
		//static_assert(std::is_integral<typename T::m_fd>::value, "This member is required");
		this->remove(obj.m_fd);
	}

	/*
	For holding events from epoll_wait() (capacity must be >= max_event).
	*/
	std::vector<struct epoll_event> m_event_vector;
private:
	int m_fd;
	static const std::size_t max_event = 20;
};

#endif
