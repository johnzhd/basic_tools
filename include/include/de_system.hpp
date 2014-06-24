#pragma once

#ifndef _MSC_VER
#include <sysconf.h>  
#else
#ifndef _WINDOWS_ 
#include <Windows.h>
#endif
#endif  

template<typename T_SIZE>
T_SIZE core_count()
{  
	T_SIZE count = 1; // ����һ��  
#ifndef _WINDOWS_
	count = sysconf(_SC_NPROCESSORS_CONF);
#else
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	count = static_cast<T_SIZE>(si.dwNumberOfProcessors);
#endif
	return count;
}

template<typename T_SIZE>
T_SIZE thread_count()
{
	size_t ret = std::thread::hardware_concurrency();
	return ret > 0? ret : 1;
}
