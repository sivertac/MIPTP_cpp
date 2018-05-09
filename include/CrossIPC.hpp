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
#include <exception>

//platform spesific
#ifdef WINDOWS
//windows stuff
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx
#include <Windows.h>

#elif LINUX
//unix stuff
#else
//error
	#error CrossIPC.hpp: Not defined target OS
#endif

namespace CrossIPC
{
	class BrokenPipeException : public std::exception
	{
	};

#ifdef WINDOWS
	std::string formatPipeError(DWORD error);
	class ErrorPipeException : public std::runtime_error
	{
	public:
		ErrorPipeException(DWORD error) :
			runtime_error("Pipe error")
		{
		}
		ErrorPipeException(std::string error) :
			runtime_error(error)
		{
		}
	};
#endif

	
	class AnonymousSocket
	{
	public:
		/*
		Constructor
		*/
		AnonymousSocket()
		{
		}
		AnonymousSocket(const std::string & pipe_string);
#ifdef WINDOWS
		AnonymousSocket(const HANDLE read_pipe, const HANDLE write_pipe);
#elif LINUX

#endif

		/*
		Write to socket
		Parameters:
			buf			buffer to write from
			len			number of bytes to write
		Return:
			size_t		number of bytes written
		*/
		std::size_t write(char* buf, std::size_t len);
		//void write(std::string & str);
		
		/*
		Read from socket
		Parameters:
			buf			buffer to read to
			buf_size	max bytes to read
		Return:
			size_t		number of bytes read
		*/
		std::size_t read(char* buf, std::size_t buf_size);
		//void read(std::string & str);

		/*
		Create a string that holds IPC descriptors (that can be used in constructor).
		Format: <m_read_pipe>-<m_write_pipe>
		*/
		std::string toString();

		/*
		Close socket (will close underlying comms)
		*/
		void close();
	
	private:

	#ifdef WINDOWS
		HANDLE m_read_pipe = NULL;
		HANDLE m_write_pipe = NULL;
	#elif LINUX
		//unix stuff
	#endif

	};

	using AnonymousSocketPair = std::pair<AnonymousSocket, AnonymousSocket>;
	/*
	Create a AnonymousSocketPair
	*/
	AnonymousSocketPair createAnonymousSocketPair();

	/*
	class NamedSocket
	{

	};
	*/
}
#endif
