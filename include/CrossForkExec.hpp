//CrossForkExec.hpp
//Author: Sivert Andresen Cubedo

#pragma once
#ifndef CrossForkExec_HEADER
#define CrossForkExec_HEADER

//common
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <exception>
//#include <memory>

#include <string.h>

//platform spesific
#ifdef WINDOWS
//windows stuff
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms682425(v=vs.85).aspx
#include <Windows.h>

#elif LINUX
//unix stuff
#else
//error
#error CrossForkExec.hpp: Not defined target OS
#endif

namespace CrossForkExec
{
#ifdef WINDOWS
	class ErrorProcessException : public std::runtime_error
	{
	public:
		ErrorProcessException(DWORD error) :
			runtime_error("Windows Process error")
		{
		}
	};
#endif

	class ChildProcess
	{
	public:
		/*
		Constructor
		*/
#ifdef WINDOWS
		ChildProcess(const PROCESS_INFORMATION & pi);
#elif LINUX

#endif

		/*
		Join child prosess
		*/
		void join();

		/*
		Close ChildProcess (kill the process)
		*/
		void close();
	private:
#ifdef WINDOWS
		PROCESS_INFORMATION m_process_information;
#elif LINUX

#endif
	};
	
	ChildProcess forkExec(const std::string & program_path, const std::vector<std::string> & program_args);

}

#endif // !CrossForkExec_HEADER
