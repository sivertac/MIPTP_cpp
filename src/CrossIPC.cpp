//CrossIPC.cpp
//Author: Sivert Andresen Cubedo

#include "../include/CrossIPC.hpp"


AnonymousSocket::AnonymousSocket() :
	m_closed(true)
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
	m_fd(fd),
	m_closed(false)
{
}
	
AnonymousSocket::AnonymousSocket(const std::string & sock_string) :
	m_fd(std::atoi(sock_string.c_str())),
	m_closed(false)
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
		throw LinuxException::BrokenPipeException();
	}
	else if (ret == -1) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			throw LinuxException::WouldBlockException();
		}
		else {
			throw LinuxException::Error("send()");
		}
	}
	return static_cast<std::size_t>(ret);
}

std::size_t AnonymousSocket::read(char* buf, std::size_t buf_size)
{
	ssize_t ret = recv(m_fd, buf, buf_size, 0);	
	if (ret == 0) {
		throw LinuxException::BrokenPipeException();
	}
	else if (ret == -1) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			throw LinuxException::WouldBlockException();
		}
		else {
			throw LinuxException::Error("recv()");
		}
	}
	return static_cast<std::size_t>(ret);
}

void AnonymousSocket::closeResources()
{
	m_closed = true;
	close(m_fd);
}

int AnonymousSocket::getFd()
{
	return m_fd;
}

bool AnonymousSocket::isClosed()
{
	return m_closed;
}

void AnonymousSocket::enableNonBlock()
{
	int flags;
	int s;
	flags = fcntl(m_fd, F_GETFL, 0);
	if (flags == -1) {
		throw LinuxException::Error("fcntl()");
	}
	flags |= O_NONBLOCK;
	s = fcntl(m_fd, F_SETFL, flags);
	if (s == -1) {
		throw LinuxException::Error("fcntl()");
	}
	m_nonblock = true;
}

void AnonymousSocket::disableNonBlock()
{
	int flags;
	int s;
	flags = fcntl(m_fd, F_GETFL, 0);
	if (flags == -1) {
		throw LinuxException::Error("fcntl()");
	}
	flags &= (~O_NONBLOCK);
	s = fcntl(m_fd, F_SETFL, flags);
	if (s == -1) {
		throw LinuxException::Error("fcntl()");
	}
	m_nonblock = false;
}

bool AnonymousSocket::isNonBlock()
{
	return m_nonblock;
}

AnonymousSocket::AnonymousSocketPair AnonymousSocket::createPair()
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

NamedSocket::NamedSocket() :
	m_closed(true)
{
}

//LINUX
NamedSocket::NamedSocket(std::string & path)
{
	std::memset(&m_sock_address, 0, sizeof(m_sock_address));
	m_sock_address.sun_family = AF_UNIX;
	nameCopy(m_sock_address, path);
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
	m_closed = false;
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
	m_closed = true;
	close(m_fd);
	unlink(m_sock_address.sun_path);
}

int NamedSocket::getFd()
{
	return m_fd;
}

bool NamedSocket::isClosed()
{
	return m_closed;
}

AnonymousSocket NamedSocket::connectToNamedSocket(std::string & path)
{
	struct sockaddr_un address;
	std::memset(&address, 0, sizeof(address));
	int connect_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (connect_fd == -1) {
		throw LinuxException::Error("socket()");
	}
	address.sun_family = AF_UNIX;
	nameCopy(address, path);
	if (connect(connect_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
		throw LinuxException::Error("connect()");
	}
	return AnonymousSocket(connect_fd);
}

void NamedSocket::nameCopy(sockaddr_un & target, std::string & source)
{
	std::memset(&target.sun_path, 0, sizeof(target.sun_path));
	if (source.size() + 1 < sizeof(target.sun_path)) {
		std::memcpy(&target.sun_path, source.c_str(), source.size() + 1);
	}
	else {
		std::memcpy(&target.sun_path, source.c_str(), sizeof(target.sun_path) - 1);
		target.sun_path[sizeof(target.sun_path) - 1] = '\0';
	}
}

AnonymousSocketPacket::AnonymousSocketPacket() :
	m_closed(true)
{
}

AnonymousSocketPacket::AnonymousSocketPacket(const int fd) :
	m_fd(fd),
	m_closed(false)
{
}

AnonymousSocketPacket::AnonymousSocketPacket(const std::string & sock_string) :
	m_fd(std::atoi(sock_string.c_str())),
	m_closed(false)
{
}

std::string AnonymousSocketPacket::toString()
{
	std::stringstream ss;
	ss << m_fd;
	return ss.str();
}

void AnonymousSocketPacket::closeResources()
{
	m_closed = true;
	close(m_fd);
}

int AnonymousSocketPacket::getFd()
{
	return m_fd;
}

bool AnonymousSocketPacket::isClosed()
{
	return m_closed;
}

void AnonymousSocketPacket::enableNonBlock()
{
	int flags;
	int s;
	flags = fcntl(m_fd, F_GETFL, 0);
	if (flags == -1) {
		throw LinuxException::Error("fcntl()");
	}
	flags |= O_NONBLOCK;
	s = fcntl(m_fd, F_SETFL, flags);
	if (s == -1) {
		throw LinuxException::Error("fcntl()");
	}
	m_nonblock = true;
}

void AnonymousSocketPacket::disableNonBlock()
{
	int flags;
	int s;
	flags = fcntl(m_fd, F_GETFL, 0);
	if (flags == -1) {
		throw LinuxException::Error("fcntl()");
	}
	flags &= (~O_NONBLOCK);
	s = fcntl(m_fd, F_SETFL, flags);
	if (s == -1) {
		throw LinuxException::Error("fcntl()");
	}
	m_nonblock = false;
}

bool AnonymousSocketPacket::isNonBlock()
{
	return m_nonblock;
}

AnonymousSocketPacket::AnonymousSocketPacketPair AnonymousSocketPacket::createPair()
{
	int fd_pair[2];
	if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, fd_pair) == -1) {
		throw LinuxException::Error("socketpair()");
	}
	return AnonymousSocketPacketPair(AnonymousSocketPacket(fd_pair[0]), AnonymousSocketPacket(fd_pair[1]));
}
