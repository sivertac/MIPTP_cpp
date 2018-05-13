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
		struct ifaddrs* interface_address;
		struct ifaddrs* it;
		if (getifaddrs(&interface_address) == -1) {
			throw std::runtime_error("getifaddrs()");
		}
		std::vector<std::string> ret_vec;
		while (it != NULL) {
			if (std::any_of(family_filter.begin(), family_filter.end(), [&](int & i) { return i == it->ifa_addr.sa_family; })) {
				ret_vec.push_back(it->ifa_name);
			}
		}
		freeifaddrs(interface_address);
		return ret_vec;
	}
#endif
}
