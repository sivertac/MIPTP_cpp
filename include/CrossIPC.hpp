//CrossIPC.hpp
//Author: Sivert Andresen Cubedo

#pragma once
#ifndef CrossIPC_HEADER
#define CrossIPC_HEADER

//includes
//common
#include <iostream>
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

	class ErrorPipeException : public std::runtime_error
	{

	};
	
	class AnonymousSocket
	{
	public:
		/*
		Constructor
		*/
#ifdef WINDOWS
		AnonymousSocket(const HANDLE read_pipe, const HANDLE write_pipe);
#elif UNIX

#endif
		/*
		Write to socket
		*/
		void write(char* buf, std::size_t len);
		void write(std::string & str);
		
		/*
		Read from socket
		*/
		void read(char* buf, std::size_t len);
		void read(std::string & str);
	
	private:

	#ifdef WINDOWS
		const HANDLE m_read_pipe;
		const HANDLE m_write_pipe;
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
