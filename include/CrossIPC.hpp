//CrossIPC.hpp
//Author: Sivert Andresen Cubedo

#pragma once
#ifndef CrossIPC_HEADER
#define CrossIPC_HEADER

//includes
//common
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <cassert>

//platform spesific
#ifdef WINDOWS
//windows stuff
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx
#include <Windows.h>
#elif LINUX
//unix stuff
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "LinuxException.hpp"

#else
//error
	#error CrossIPC.hpp: Not defined target OS
#endif


class BrokenPipeException : public std::exception
{
};
	
class AnonymousSocket
{
public:
	/*
	Constructor.
	*/
	AnonymousSocket();

	/*
	Constructor.
	Parameters:
		sock_string		ref to string
	*/
	AnonymousSocket(const std::string & sock_string);
#ifdef WINDOWS
	/*
	Constructor.
	Parameters:
		read_pipe		HANDLE to read pipe
		write_pipe		HANDLE to write pipe
	*/
	AnonymousSocket(const HANDLE read_pipe, const HANDLE write_pipe);
#elif LINUX
	AnonymousSocket(const int fd);
#endif

	/*
	Write to socket.
	Parameters:
		buf			buffer to write from
		len			number of bytes to write
	Return:
		size_t		number of bytes written
	*/
	std::size_t write(const char* buf, std::size_t len);
		
	/*
	Write string to socket.
	Parameters:
		str			ref to string
	Return:		
		void
	*/
	void writeString(const std::string & str);

	/*
	Read from socket
	Parameters:
		buf			buffer to read to
		buf_size	max bytes to read
	Return:		
		size_t		number of bytes read
	*/
	std::size_t read(char* buf, std::size_t buf_size);
		
	/*
	Read string from socket.
	Parameters:
	Return:		
		std::string
	*/
	std::string readString();

	/*
	Create a string that holds IPC descriptors (that can be used in constructor).
	Format: <m_read_pipe>-<m_write_pipe>
	Parameters:
	Return:		
		std::string
	*/
	std::string toString();

	/*
	Close socket (will close underlying comms)
	Parameters:
	Return:	
		void
	*/
	void closeResources();

	template <typename T>
	friend AnonymousSocket & operator<<(AnonymousSocket & sock, const T & source)
	{
		std::size_t ret = sock.write((char*)&source, sizeof(T));
		return sock;			
	}
	template <typename T>
	friend AnonymousSocket & operator<<(AnonymousSocket & sock, const T * source)
	{
		std::size_t ret = sock.write((char*)source, sizeof(T));
		return sock;
	}
	template <typename T>
	friend AnonymousSocket & operator>>(AnonymousSocket & sock, T & dest)
	{
		std::size_t ret = sock.read((char*)&dest, sizeof(T));
		return sock;
	}

	using AnonymousSocketPair = std::pair<AnonymousSocket, AnonymousSocket>;
	/*
	Create a AnonymousSocketPair.
	Parameters:
	Return:
		pair
	*/
	static AnonymousSocketPair createAnonymousSocketPair();

private:

#ifdef WINDOWS
	HANDLE m_read_pipe = NULL;
	HANDLE m_write_pipe = NULL;
#elif LINUX
	int m_fd;
#endif
};
	
class NamedSocket
{
public:
	/*
	Constructor.
	*/
	NamedSocket(std::string & path);

	/*
	Accept connection (will block).
	Parameters:
	Return:
		anon socket
	*/
	AnonymousSocket acceptConnection();

	/*
	Close (release resources).
	Parameters:
	Return:
		void
	*/
	void closeResources();

	/*
	Connect to a named socket.
	Parameters:
		path		name of socket
	Return:
		anon socket
	*/
	static AnonymousSocket connectToNamedSocket(std::string & path);

private:
	int m_fd;
	struct sockaddr_un m_sock_address;
};

#endif
