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

#elif UNIX
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
#endif

	class ErrorPipeException : public std::runtime_error
	{
	public:
		ErrorPipeException(DWORD error) :
			std::runtime_error("Pipe error")
		{
		}
	};
	
	class AnonymousSocket
	{
	public:
		/*
		Constructor
		*/
#ifdef WINDOWS
		AnonymousSocket(const PHANDLE read_pipe, const PHANDLE write_pipe);
#elif UNIX

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
	
	private:

	#ifdef WINDOWS
		const PHANDLE m_read_pipe;
		const PHANDLE m_write_pipe;
	#elif UNIX
		//unix stuff
	#endif

	};

	/*
	Create a AnonymousSocketPair
	*/
	using AnonymousSocketPair = std::pair<AnonymousSocket, AnonymousSocket>;
	AnonymousSocketPair createAnonymousSocketPair();

	/*
	class NamedSocket
	{

	};
	*/
}
#endif
