//TimerWrapper.cpp
//Author: Sivert Andresen Cubedo

#include "../include/TimerWrapper.hpp"

TimerWrapper::TimerWrapper() :
	m_closed(false)
{
	m_fd = timerfd_create(CLOCK_REALTIME, 0);
	if (m_fd == -1) {
		m_closed = true;
		throw LinuxException::Error("timerfd_create()");
	}
}

void TimerWrapper::setExpirationFromNow(int ms)
{
	struct timespec now_time;
	struct timespec in_time = msToTimespec(ms);
	struct itimerspec spec;
	if (clock_gettime(CLOCK_REALTIME, &now_time) == -1) {
		LinuxException::Error("clock_gettime()");
	}
	spec.it_interval.tv_sec = 0;
	spec.it_interval.tv_nsec = 0;
	spec.it_value.tv_sec = now_time.tv_sec + in_time.tv_sec;
	spec.it_value.tv_nsec = now_time.tv_nsec + in_time.tv_nsec;
	if (timerfd_settime(m_fd, TFD_TIMER_ABSTIME, &spec, NULL) == -1) {
		throw LinuxException::Error("timerfd_settime()");
	}
}

int TimerWrapper::readExpiredTime()
{
	std::uint64_t exp;
	ssize_t ret = read(m_fd, &exp, sizeof(exp));
	if (ret != sizeof(exp)) {
		LinuxException::Error("read()");
	}
	return static_cast<int>(exp);
}

int TimerWrapper::getFd()
{
	return m_fd;
}

void TimerWrapper::closeResources()
{
	close(m_fd);
	m_closed = true;
}

bool TimerWrapper::isClosed()
{
	return m_closed;
}

int TimerWrapper::timespecToMs(timespec & t)
{
	int ms;
	ms = t.tv_sec * 1000;
	ms += t.tv_nsec / 1000000;
	return ms;
}

timespec TimerWrapper::msToTimespec(int ms)
{
	struct timespec t;
	t.tv_sec = ms / 1000;
	t.tv_nsec = (ms % 1000) * 1000000;
	return t;
}
