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

	void ChildProcess::closeResources()
	{
		CloseHandle(m_process_information.hProcess);
		CloseHandle(m_process_information.hThread);
	}

	ChildProcess forkExec(const std::string & program_path, const std::vector<std::string> & program_args)
	{
		STARTUPINFO startup_info;
		std::memset(&startup_info, 0, sizeof(STARTUPINFO));
		PROCESS_INFORMATION process_information;
		std::memset(&process_information, 0, sizeof(PROCESS_INFORMATION));

		std::ostringstream cmd_stream;
		cmd_stream << program_path;
		for (std::string s : program_args) {
			cmd_stream << " " << s;
		}

		std::string cmd_string = cmd_stream.str();
		std::size_t len = cmd_string.length() + 1; //+1 for \0
		
		LPSTR cmd = (LPSTR)std::malloc(len);	
		if (cmd == NULL) {
			throw std::runtime_error("forkExec: std::malloc()");
		}

		strncpy_s(cmd, len, cmd_string.c_str(), len);

		if (!CreateProcess(
			NULL,
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

		std::free(cmd);

		return ChildProcess(process_information);
	}

#elif LINUX
	
	ChildProcess::ChildProcess(const pid_t & process_id) :
		m_process_id(process_id)
	{
	}
	
	void ChildProcess::join()
	{
		int status;
		if (waitpid(m_process_id, &status, 0) == -1) {
			throw std::runtime_error("wairpid()");
		}
	}

	void ChildProcess::closeResources() {
		close(m_process_id);
	}

	ChildProcess forkExec(const std::string & program_path, const std::vector<std::string> & program_args)
	{
		pid_t process_id = fork();
		if (process_id == -1) {
			throw std::runtime_error("fork()");
		}
		else if (process_id == 0) {
			char** argv = (char**)std::malloc((program_args.size() + 2) * sizeof(char*));
			if (argv == NULL) {
				throw std::runtime_error("malloc()");
			}
			argv[0] = strdup(program_path.c_str());
			for (std::size_t i = 0; i < program_args.size(); ++i) {
				argv[i + 1] = strdup(program_args[i].c_str());
			}
			argv[program_args.size() + 1] = NULL;
			if (execvp(argv[0], argv) == -1) {
				throw std::runtime_error("execvp()");
			}
			// -> child should not come here <-
		}
		return ChildProcess(process_id);
	}
#endif
}
