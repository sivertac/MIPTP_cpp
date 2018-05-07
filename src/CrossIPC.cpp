//CrossIPC.cpp
//Author: Sivert Andresen Cubedo

#include "../include/CrossIPC.hpp"

namespace CrossIPC
{
#ifdef WINDOWS
	AnonymousSocket::AnonymousSocket(const HANDLE read_pipe, const HANDLE write_pipe) :
		m_read_pipe(read_pipe),
		m_write_pipe(write_pipe)
	{
	}

	void AnonymousSocket::write(char* buf, std::size_t len)
	{
		
		if (!WriteFile(m_write_pipe, buf, len, NULL, NULL)) {
			DWORD error = GetLastError();
			if (error == ERROR_BROKEN_PIPE) {
				throw BrokenPipeException();
			}
		}
	}

	void AnonymousSocket::read(char* buf, std::size_t len)
	{
		
	}
	
	
#elif UNIX

#endif

}




