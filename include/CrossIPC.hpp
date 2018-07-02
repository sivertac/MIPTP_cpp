//CrossIPC.hpp
//Author: Sivert Andresen Cubedo

#pragma once
#ifndef CrossIPC_HEADER
#define CrossIPC_HEADER

//C++
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
#include <fcntl.h>

#include "LinuxException.hpp"

#else
//error
	#error CrossIPC.hpp: Not defined target OS
#endif

/*
Unix socket wrapper with stream interface.
*/
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
	Generic write (write sizeof(T)).
	Parameters:
		data		ref to data
	Return:
		size_t		number of bytes read
	*/
	template <typename T>
	std::size_t writeGeneric(T & data)
	{
		return write(reinterpret_cast<char*>(&data), sizeof(T));
	}

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
	Generic read (read sizeof(T)).
	Parameters:
		data		ref to data
	Return:
		size_t		number of bytes read
	*/
	template <typename T>
	std::size_t readGeneric(T & data)
	{
		return read(reinterpret_cast<char*>(&data), sizeof(T));
	}
		
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

	/*
	Get fd.
	Parameters:
	Return:
		fd
	*/
	int getFd();

	/*
	Check if sock is closed.
	Parameters:
	Return:
		if true close, else valid
	*/
	bool isClosed();

	/*
	Enable nonblocking.
	Parameters:
	Return:
		void
	*/
	void enableNonBlock();

	/*
	Disable nonblocking.
	Parameters:
	Return:
		void
	*/
	void disableNonBlock();

	/*
	Check if nonblocking
	Parameters:
	Return:
		bool
	*/
	bool isNonBlock();

	using AnonymousSocketPair = std::pair<AnonymousSocket, AnonymousSocket>;
	/*
	Create a AnonymousSocketPair.
	Parameters:
	Return:
		pair
	*/
	static AnonymousSocketPair createPair();

private:
#ifdef WINDOWS
	HANDLE m_read_pipe = NULL;
	HANDLE m_write_pipe = NULL;
#elif LINUX
	int m_fd;
#endif
	bool m_closed;
	bool m_nonblock;
};

class NamedSocket
{
public:
	/*
	Constructor.
	*/
	NamedSocket();

	/*
	Constructor.
	Parameters:
		path		name of socket
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
	Get fd.
	Parameters:
	Return:
		fd
	*/
	int getFd();

	/*
	Check if sock is closed.
	Parameters:
	Return:
		if true close, else valid
	*/
	bool isClosed();

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
	bool m_closed;
	struct sockaddr_un m_sock_address;

	/*
	Name copy mechanism.
	Parameters:
		target
		source
	Return:
		void
	*/
	static void nameCopy(struct sockaddr_un & target, std::string & source);
};

/*
Unix socket wrapper with SEQ_PACKET interface.
*/
class AnonymousSocketPacket
{
public:
	/*
	struct iovec wrapper.
	*/
	template <std::size_t N>
	class IovecWrapper
	{
	public:
		/*
		Set index in iovec with pointer and custom size.
		Parameters:
			index
			ptr
			size
		Return:
			void
		*/
		void setIndex(std::size_t index, void* ptr, std::size_t size)
		{
			assert(index < N);
			m_iov[index].iov_base = ptr;
			m_iov[index].iov_len = size;
		}

		/*
		Set index in iovec with pointer to of ref and size of T.
		Parameters:
			index	index to set
			data	ref to data to point to
		Return:
			void
		*/
		template <typename T>
		void setIndex(std::size_t index, T & data)
		{
			setIndex(index, reinterpret_cast<void*>(&data), sizeof(T));
		}

		/*
		Get size.
		Parameters:
		Return
			size
		*/
		std::size_t getSize() {
			return N;
		}

		struct iovec m_iov[N];
	};

	/*
	Constructor.
	*/
	AnonymousSocketPacket();

	/*
	Constructor:
	Parameters:
		fd
	*/
	AnonymousSocketPacket(const int fd);

	/*
	Constructor:
	Parameters:
		sock_string
	*/
	AnonymousSocketPacket(const std::string & sock_string);

	/*
	Send iovec.
	Parameters:
		iov			ref to iovec
		iov_size	iovec size
	Return:
		void
	*/
	template <std::size_t N>
	void sendiovec(IovecWrapper<N> & iov)
	{
		struct msghdr message;
		std::memset(&message, 0, sizeof(message));
		message.msg_iov = iov.m_iov;
		message.msg_iovlen = iov.getSize();
		ssize_t ret = sendmsg(m_fd, &message, 0);
		if (ret == 0) {
			throw LinuxException::BrokenPipeException();
		}
		if (ret == -1) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				throw LinuxException::WouldBlockException();
			}
			else {
				throw LinuxException::Error("sendmsg()");
			}
		}
	}

	/*
	Receive iovec.
	Parameters:
		iov			ref to iovec
		iov_size	iovec_size
	Return:
		void
	*/
	template <std::size_t N>
	void recviovec(IovecWrapper<N> & iov)
	{
		struct msghdr message;
		std::memset(&message, 0, sizeof(message));
		message.msg_iov = iov.m_iov;
		message.msg_iovlen = iov.getSize();
		ssize_t ret = recvmsg(m_fd, &message, 0);
		if (ret == 0) {
			throw LinuxException::BrokenPipeException();
		}
		if (ret == -1) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				throw LinuxException::WouldBlockException();
			}
			else {
				throw LinuxException::Error("sendmsg()");
			}
		}
	}

	/*
	Create a string that holds IPC descriptors (that can be used in constructor).
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

	/*
	Get fd.
	Parameters:
	Return:
		fd
	*/
	int getFd();

	/*
	Check if sock is closed.
	Parameters:
	Return:
		if true close, else valid
	*/
	bool isClosed();

	/*
	Enable nonblocking.
	Parameters:
	Return:
		void
	*/
	void enableNonBlock();

	/*
	Disable nonblocking.
	Parameters:
	Return:
		void
	*/
	void disableNonBlock();

	/*
	Check if nonblocking
	Parameters:
	Return:
		bool
	*/
	bool isNonBlock();

	using AnonymousSocketPacketPair = std::pair<AnonymousSocketPacket, AnonymousSocketPacket>;
	/*
	Create a AnonymousSocketPacketPair.
	Parameters:
	Return:
		pair
	*/
	static AnonymousSocketPacketPair createPair();
private:
	int m_fd;
	bool m_closed;
	bool m_nonblock;
};

#endif
