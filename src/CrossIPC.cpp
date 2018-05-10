//CrossIPC.cpp
//Author: Sivert Andresen Cubedo

#include "../include/CrossIPC.hpp"

namespace CrossIPC
{
#ifdef WINDOWS

	std::string formatPipeError(DWORD error)
	{
		std::ostringstream s;
		s << "Windows pipe error: ";
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

	AnonymousSocket::AnonymousSocket(const std::string & pipe_string)
	{
		std::size_t mid_index = pipe_string.find_first_of('-');
		if (mid_index == pipe_string.npos) {
			throw ErrorPipeException("Invalid pipe string");
		}
		std::string read_str = pipe_string.substr(0, mid_index);
		std::string write_str = pipe_string.substr(mid_index + 1, pipe_string.length() - (mid_index + 1));
		std::stringstream ss_read(read_str);
		std::stringstream ss_write(write_str);
		UINT_PTR read_ptr;
		UINT_PTR write_ptr;
		ss_read >> read_ptr;
		ss_write >> write_ptr;
		m_read_pipe = reinterpret_cast<HANDLE>(read_ptr);
		m_write_pipe = reinterpret_cast<HANDLE>(write_ptr);
	}

	AnonymousSocket::AnonymousSocket(const HANDLE read_pipe, const HANDLE write_pipe) :
		m_read_pipe(read_pipe),
		m_write_pipe(write_pipe)
	{
	}

	std::size_t AnonymousSocket::write(const char* buf, std::size_t len)
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

	std::string AnonymousSocket::toString()
	{
		std::stringstream ss;
		ss << reinterpret_cast<std::size_t>(m_read_pipe) << "-" << reinterpret_cast<std::size_t>(m_write_pipe);
		return ss.str();
	}

	void AnonymousSocket::close()
	{
		CloseHandle(m_read_pipe);
		m_read_pipe = NULL;
		CloseHandle(m_write_pipe);
		m_write_pipe = NULL;
	}
		
#elif LINUX

#endif
	void AnonymousSocket::writeString(const std::string & str)
	{
		std::size_t ret = write(str.c_str(), str.length());
	}
	std::string AnonymousSocket::readString()
	{
		const std::size_t buf_size = 1024;
		char buf[buf_size];
		std::stringstream ss;
		std::size_t ret;
		do {
			ret = read(buf, buf_size - 1);
			buf[ret] = '\0';
			ss << buf;
		} while (ret >= buf_size - 1);
		return ss.str();
	}
}




