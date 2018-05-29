//TimerWrapper.hpp
//Author: Sivert Andresen Cubedo

#pragma once
#ifndef TimerWrapper_HEADER
#define TimerWrapper_HEADER

#include <iostream>
#include <sstream>

//Linux stuff
#include <sys/timerfd.h>
#include <unistd.h>

//local
#include "LinuxException.hpp"

class TimerWrapper
{
public:
	/*
	Constructor.
	*/
	TimerWrapper();

	/*
	Set expiration from now.
	Parameters:
		ms			time
	Return:
		void
	*/
	void setExpirationFromNow(int ms);

	/*
	Read expired from fd.
	Parameters:
	Return:
		number of times the timer has expired since armed
	*/
	int readExpiredTime();
	
	/*
	Get fd.
	Parameters:
	Return:
		fd
	*/
	int getFd();
private:
	int m_fd;

	/*
	timespec to ms.
	*/
	int timespecToMs(struct timespec & t);
	
	/*
	ms to timespec.
	*/
	struct timespec msToTimespec(int ms);
};

#endif // !TimerWrapper_HEADER
