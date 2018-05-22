//CrossIPC.cpp
//Author: Sivert Andresen Cubedo

#include "../include/CrossIPC.hpp"

namespace CrossIPC
{
	AnonymousSocket::AnonymousSocket()
	{
	}
#ifdef WINDOWS
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
			throw ErrorPipeException();
		}

		if (!CreatePipe(&read_2, &write_1, &sa_attr, 0)) {
			throw ErrorPipeException();
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
				throw ErrorPipeException();
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
				throw ErrorPipeException();
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

	void AnonymousSocket::closeResources()
	{
		CloseHandle(m_read_pipe);
		m_read_pipe = NULL;
		CloseHandle(m_write_pipe);
		m_write_pipe = NULL;
	}
		
#elif LINUX
	AnonymousSocket::AnonymousSocket(const int fd) :
		m_fd(fd)
	{
	}
	
	AnonymousSocket::AnonymousSocket(const std::string & sock_string) :
		m_fd(std::atoi(sock_string.c_str()))
	{
	}
	
	std::string AnonymousSocket::toString()
	{
		std::stringstream ss;
		ss << m_fd;
		return ss.str();
	}
	
	std::size_t AnonymousSocket::write(const char* buf, std::size_t len)
	{
		ssize_t ret = send(m_fd, buf, len, 0);
		if (ret == 0) {
			throw BrokenPipeException();
		}
		else if (ret == -1) {
			throw LinuxException::Error("send()");
		}
		return static_cast<std::size_t>(ret);
	}

	std::size_t AnonymousSocket::read(char* buf, std::size_t buf_size)
	{
		ssize_t ret = recv(m_fd, buf, buf_size, 0);
		if (ret == 0) {
			throw BrokenPipeException();
		}
		else if (ret == -1) {
			throw LinuxException::Error("recv()");
		}
		return static_cast<std::size_t>(ret);
	}

	void AnonymousSocket::closeResources()
	{
		close(m_fd);
	}

	AnonymousSocketPair createAnonymousSocketPair()
	{
		int fd_pair[2];
		if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd_pair) == -1) {
			throw LinuxException::Error("socketpair()");
		}
		return AnonymousSocketPair(AnonymousSocket(fd_pair[0]), AnonymousSocket(fd_pair[1]));
	}

#endif
	
	void AnonymousSocket::writeString(const std::string & str)
	{
		write(str.c_str(), str.length());
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

	//LINUX
	NamedSocket::NamedSocket(std::string & path)
	{
		assert(path.size() < sizeof(m_sock_address.sun_path) - 1);
		m_sock_address.sun_family = AF_UNIX;
		std::memset(m_sock_address.sun_path, 0, sizeof(m_sock_address.sun_path));
		std::strcpy(m_sock_address.sun_path, path.c_str());
		m_fd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (m_fd == -1) {
			throw LinuxException::Error("socket()");
		}
		if (bind(m_fd, (struct sockaddr*)&m_sock_address, sizeof(struct sockaddr_un)) == -1) {
			throw LinuxException::Error("bind()");
		}
		if (listen(m_fd, 100) == -1) {
			throw LinuxException::Error("listen()");
		}
	}

	AnonymousSocket NamedSocket::acceptConnection()
	{
		struct sockaddr_un address;
		std::memset(&address, 0, sizeof(address));
		address.sun_family = AF_UNIX;
		socklen_t address_len = sizeof(address);
		int accept_fd = accept(m_fd, (struct sockaddr*)&m_sock_address, &address_len);
		if (accept_fd == -1) {
			throw LinuxException::Error("accept()");
		}
		return AnonymousSocket(accept_fd);
	}

	void NamedSocket::closeResources()
	{
		close(m_fd);
		unlink(m_sock_address.sun_path);
	}

	AnonymousSocket connectToNamedSocket(std::string & path)
	{
		struct sockaddr_un address;
		assert(path.size() < sizeof(address.sun_path) - 1);
		int connect_fd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (connect_fd == -1) {
			throw LinuxException::Error("socket()");
		}
		std::memset(&address, 0, sizeof(address));
		address.sun_family = AF_UNIX;
		std::strcpy(address.sun_path, path.c_str());
		if (connect(connect_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
			throw LinuxException::Error("connect()");
		}
		return AnonymousSocket(connect_fd);
	}
}



