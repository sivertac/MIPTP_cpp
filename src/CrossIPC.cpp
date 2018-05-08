//CrossIPC.cpp
//Author: Sivert Andresen Cubedo

#include "../include/CrossIPC.hpp"

namespace CrossIPC
{
#ifdef WINDOWS

	std::string formatPipeError(DWORD error)
	{
		std::ostringstream s("Windows pipe error: ");
		s << error;
		return s.str();
	}

	AnonymousSocketPair createAnonymousSocketPair()
	{
		SECURITY_ATTRIBUTES sa_attr;
		sa_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
		std::memset(&sa_attr, 0, sizeof(SECURITY_ATTRIBUTES));
		sa_attr.bInheritHandle = TRUE;
		sa_attr.lpSecurityDescriptor = NULL;
		
		HANDLE write_1 = NULL;
		HANDLE write_2 = NULL;
		HANDLE read_1 = NULL;
		HANDLE read_2 = NULL;

		if (!CreatePipe(&read_1, &write_2, &sa_attr, 0)) {
			throw ErrorPipeException(GetLastError());
		}

		if (!CreatePipe(&read_2, &write_1, &sa_attr, 0)) {
			throw ErrorPipeException(GetLastError());
		}
		return AnonymousSocketPair(AnonymousSocket(read_1, write_1), AnonymousSocket(read_2, write_2));
	}

	AnonymousSocket::AnonymousSocket(const HANDLE read_pipe, const HANDLE write_pipe) :
		m_read_pipe(read_pipe),
		m_write_pipe(write_pipe)
	{
	}

	std::size_t AnonymousSocket::write(char* buf, std::size_t len)
	{
		DWORD written_len;
		if (!WriteFile(m_write_pipe, buf, static_cast<DWORD>(len), &written_len, NULL)) {
			DWORD error = GetLastError();
			if (error == ERROR_BROKEN_PIPE) {
				throw BrokenPipeException();
			}
			else {
				throw ErrorPipeException(error);
			}
		}
		return static_cast<std::size_t>(written_len);
	}

	std::size_t AnonymousSocket::read(char* buf, std::size_t buf_size)
	{
		DWORD read_len;
		if (!ReadFile(m_read_pipe, buf, static_cast<DWORD>(buf_size), &read_len, NULL)) {
			DWORD error = GetLastError();
			if (error == ERROR_BROKEN_PIPE) {
				throw BrokenPipeException();
			}
			else {
				throw ErrorPipeException(error);
			}
		}
		return static_cast<std::size_t>(read_len);
	}

	void AnonymousSocket::close()
	{
		CloseHandle(m_read_pipe);
		CloseHandle(m_write_pipe);
	}
	
	
#elif LINUX

#endif

}




