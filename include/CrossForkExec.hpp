//CrossForkExec.hpp
//Author: Sivert Andresen Cubedo

#pragma once
#ifndef CrossForkExec_HEADER
#define CrossForkExec_HEADER

//common
#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>
#include <exception>
#include <stdexcept>
//#include <memory>

#include <string.h>

//platform spesific
#ifdef WINDOWS
//windows stuff
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms682425(v=vs.85).aspx
#include <Windows.h>

#elif LINUX
//unix stuff
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "LinuxException.hpp"

#else
//error
#error CrossForkExec.hpp: Not defined target OS
#endif

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
	Constructor.
	*/
	ChildProcess();
#ifdef WINDOWS
	/*
	Constructor.
	Paremters:
		pi
	*/
	ChildProcess(const PROCESS_INFORMATION & pi);
#elif LINUX
	/*
	Constructor.
	Parameters:
		process_id
	*/
	ChildProcess(const pid_t & process_id);
#endif

	/*
	Join child prosess.
	Parameters:
	Return:
		void
	*/
	void join();

	/*
	Close ChildProcess (close the handlers).
	Parameters:
	Return:
		void
	*/
	void closeResources();

	/*
	Fork exec.
	Parameters:
		program_path		path to binary
		program_args		arguments
	Return:
		child process
	*/
	static ChildProcess forkExec(const std::string & program_path, const std::vector<std::string> & program_args);
private:
#ifdef WINDOWS
	PROCESS_INFORMATION m_process_information;
#elif LINUX
	pid_t m_process_id;
#endif
};

#endif // !CrossForkExec_HEADER
