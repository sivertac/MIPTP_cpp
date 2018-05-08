//CrossForkExec.cpp
//Author: Sivert Andresen Cubedo

#include "../include/CrossForkExec.hpp"

namespace CrossForkExec
{
#ifdef WINDOWS
	ChildProcess::ChildProcess(PROCESS_INFORMATION & pi) :
		m_process_information(pi)
	{
	}

	void ChildProcess::join()
	{
		WaitForSingleObject(m_process_information.hProcess, INFINITE);
	}

	void ChildProcess::close()
	{
		CloseHandle(m_process_information.hProcess);
		CloseHandle(m_process_information.hThread);
	}

	ChildProcess forkExec(const std::string & program_path, const std::vector<std::string>& program_args)
	{
		STARTUPINFO startup_info;
		PROCESS_INFORMATION process_information;



		//return ChildProcess();
	}

#elif LINUX

#endif


}
