//CrossIPC.hpp
//Author: Sivert Andresen Cubedo

#pragma once
#ifndef CrossIPC_HEADER
#define CrossIPC_HEADER

//includes
#ifdef WINDOWS
//windows stuff
#include <iostream>
#include <string>
#include <Windows.h>

#elif UNIX
//unix stuff
#else
//error
	#error CrossIPC.hpp: Not defined target OS
#endif

namespace CrossIPC
{
	 

	class AnonymousSocket
	{
	public:
		/*
		Read from socket
		*/
		void read(char* buf, std::size_t len);
		void read(std::string & str);

		/*
		Write to socket
		*/
		void write(char* buf, std::size_t len);
		void write(std::string & str);
	
	private:

	#ifdef WINDOWS

	#elif UNIX
		//unix stuff
	#else
		//error
		#error CrossIPC.hpp: Not defined target OS
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
