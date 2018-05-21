//RawSock.cpp
//Author: Sivert Andresen Cubedo

#include "../include/RawSock.hpp"

namespace RawSock
{
#ifdef WINDOWS
	/*
	std::vector<std::wstring> getInterfaceNames()
	{
		//https://msdn.microsoft.com/en-us/library/windows/desktop/aa365947(v=vs.85).aspx
		PIP_INTERFACE_INFO info = NULL;
		ULONG info_buf_len = 0;
		DWORD ret = 0;
		ret = GetInterfaceInfo(NULL, &info_buf_len);
		if (ret == ERROR_INSUFFICIENT_BUFFER) {
			info = (IP_INTERFACE_INFO*)std::malloc(info_buf_len);
			if (info == NULL) {
				throw std::runtime_error("malloc()");
			}
		}
		
		ret = GetInterfaceInfo(info, &info_buf_len);
		std::vector<std::wstring> name_vec;

		if (ret == NO_ERROR) {
			name_vec.reserve(info->NumAdapters);
			for (std::size_t i = 0; i < info->NumAdapters; ++i) {
				name_vec.push_back(info->Adapter[i].Name);
			}
		}
		else {
			throw std::runtime_error("GetInterfaceInfo()");
		}

		std::free(info);

		return name_vec;
	}
	*/
#elif LINUX
	std::vector<std::string> getInterfaceNames(const std::vector<int> & family_filter)
	{
		struct ifaddrs* start_it;
		struct ifaddrs* it;
		if (getifaddrs(&start_it) == -1) {
			throw LinuxException::Error("getifaddrs()");
		}
		std::vector<std::string> ret_vec;
		it = start_it;
		while (it != NULL) {
			if (std::any_of(family_filter.begin(), family_filter.end(), [&](const int & i) { return i == it->ifa_addr->sa_family; })) {
				ret_vec.push_back(it->ifa_name);
			}
			it = it->ifa_next;
		}
		freeifaddrs(start_it);
		return ret_vec;
	}

	MACAddress getMacAddress(int fd, const std::string & interface_name)
	{
		struct ifreq dev;
		strcpy(dev.ifr_name, interface_name.c_str());
		if (ioctl(fd, SIOCGIFHWADDR, &dev) == -1) {
			throw LinuxException::Error("ioctl()");
		}

		MACAddress mac;
		std::memcpy(mac.data(), dev.ifr_hwaddr.sa_data, 6);
		return mac;
	}

	void setNonBlocking(int fd)
	{
		int flags;
		int s;
		flags = fcntl(fd, F_GETFL, 0);
		if (flags == -1) {
			throw LinuxException::Error("fcntl()");
		}
		flags |= O_NONBLOCK;
		s = fcntl(fd, F_SETFL, flags);
		if (s == -1) {
			throw LinuxException::Error("fcntl()");
		}
	}

	MIPRawSock::MIPRawSock(const std::string & interface_name, MIPAddress mip)
	{
		int protocol = htons(ETH_P_MIP);
		m_mip = mip;
		m_fd = socket(AF_PACKET, SOCK_RAW, protocol);
		if (m_fd == -1) {
			throw LinuxException::Error("socket()");
		}
		m_mac = getMacAddress(m_fd, interface_name);
		m_sock_address.sll_family = AF_PACKET;
		m_sock_address.sll_protocol = protocol;
		m_sock_address.sll_ifindex = if_nametoindex(interface_name.c_str());
		if (bind(m_fd, (struct sockaddr*)&m_sock_address, sizeof(m_sock_address)) == -1) {
			throw LinuxException::Error("bind()");
		}
	}

	void MIPRawSock::closeResources()
	{
		close(m_fd);
	}

	void MIPRawSock::sendEthernetFrame(EthernetFrame & frame)
	{
		ssize_t ret = send(m_fd, frame.getData(), frame.getSize(), 0);
		if (ret == -1) {
			throw LinuxException::Error("send()");
		}
	}

	void MIPRawSock::recvEthernetFrame(EthernetFrame & frame)
	{
		if (frame.getSize() < EthernetFrame::FRAME_MAX_SIZE) {
			frame.setSize(EthernetFrame::FRAME_MAX_SIZE);
		}
		ssize_t ret = recv(m_fd, frame.getVector().data(), EthernetFrame::FRAME_MAX_SIZE, 0);
		if (ret == -1) {
			throw LinuxException::Error("recv()");
		}
		frame.setSize(static_cast<std::size_t>(ret));
	}

	int MIPRawSock::getFd()
	{
		return m_fd;
	}

	MIPAddress MIPRawSock::getMip()
	{
		return m_mip;
	}

	MACAddress MIPRawSock::getMac()
	{
		return m_mac;
	}

	std::string MIPRawSock::toString()
	{
		std::ostringstream ss;
		ss	<< "fd: " << getFd() << "\n"
			<< "mip: " << toStringMIPAddress(getMip()) << "\n"
			<< "mac: " << toStringMACAddress(getMac());
		return ss.str();
	}

#endif
}
