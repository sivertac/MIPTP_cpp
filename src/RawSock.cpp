//RawSock.cpp
//Author: Sivert Andresen Cubedo

#include "../include/RawSock.hpp"

namespace RawSock
{
#ifdef WINDOWS
	std::vector<std::string> getInterfaceNames()
	{
		//https://msdn.microsoft.com/en-us/library/windows/desktop/aa365947(v=vs.85).aspx
		PIP_INTERFACE_INFO info;
		ULONG info_buf_len = 0;
		DWORD ret;

		ret = GetInterfaceInfo(NULL, &info_buf_len);
		if (ret == ERROR_INSUFFICIENT_BUFFER) {
			info = (IP_INTERFACE_INFO*)std::malloc(info_buf_len);
			if (info == NULL) {
				throw std::runtime_error("malloc()");
			}
		}


		return std::vector<std::string>();
	}
#elif LINUX

#endif
}
