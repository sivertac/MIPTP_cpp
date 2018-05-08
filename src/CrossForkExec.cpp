//CrossForkExec.cpp
//Author: Sivert Andresen Cubedo

#include "../include/CrossForkExec.hpp"

namespace CrossForkExec
{
#ifdef WINDOWS
	ChildProcess::ChildProcess(const PROCESS_INFORMATION & pi) :
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

	ChildProcess forkExec(const std::string & program_path, const std::vector<std::string> & program_args)
	{
		STARTUPINFO startup_info;
		PROCESS_INFORMATION process_information;

		std::ostringstream cmd_stream;
		cmd_stream << program_path;
		for (std::string s : program_args) {
			cmd_stream << " " << s;
		}

		//std::size_t len = program_path.length + 1;
		//LPCSTR app = new char[len];
		//strncpy_s(app, len, program_path.c_str(), len);

		std::string cmd_string = cmd_stream.str();
		len = cmd_string.length() + 1; //+1 for \0
		LPSTR cmd = new char[len];	
		strncpy_s(cmd, len, cmd_string.c_str(), len);		


		std::cout << cmd << "\n";

		if (!CreateProcess(,
			cmd,
			NULL,
			NULL,
			TRUE,
			0,
			NULL,
			NULL,
			&startup_info,
			&process_information
		)) {
			throw ErrorProcessException(GetLastError());
		}

		free(cmd);

		return ChildProcess(process_information);
	}

#elif LINUX

#endif


}
