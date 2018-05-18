//Author: Sivert Andresen Cubedo
//EventPoll.hpp

#pragma once
#ifndef EventPoll_HEADER
#define EventPoll_HEADER

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
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
		vec		ref to vector<int> that will be filled with file_descriptors (will clear data)
	Return:
		true if the wait was valid
		false if the epoll is closed
	*/
	bool wait(std::vector<int> & vec);

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

private:
	int m_fd;
	const std::size_t max_events = 20;
};

#endif
