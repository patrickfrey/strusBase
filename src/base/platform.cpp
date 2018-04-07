/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/platform.hpp"

#define PLATFORM_UNKNOWN 0
#define PLATFORM_CPP11 1
#define PLATFORM_WINDOWS 2
#define PLATFORM_BSD 3
#define PLATFORM_LINUX 4

// Evaluate platform:

#if __cplusplus >= 201103L
#define PLATFORM PLATFORM_CPP11
#include <thread>

#elif defined(_MSC_VER)
#include <windows.h>
#define PLATFORM PLATFORM_WINDOWS

#elif defined(__linux__)
#include <unistd.h>
#define PLATFORM PLATFORM_LINUX

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#define PLATFORM PLATFORM_BSD

#else
#define PLATFORM PLATFORM_UNKNOWN
#endif

using namespace strus;
using namespace strus::platform;

int platform::cores()
{
#if PLATFORM==PLATFORM_CPP11
	return std::thread::hardware_concurrency();
#elif PLATFORM==PLATFORM_WINDOWS
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#elif PLATFORM==PLATFORM_LINUX
	return sysconf(_SC_NPROCESSORS_ONLN);
#elif PLATFORM==PLATFORM_BSD
	int mib[4];
	int numCPU;
	std::size_t len = sizeof(numCPU); 
	
	/* set the mib for hw.ncpu */
	mib[0] = CTL_HW;
	mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;
	
	/* get the number of CPUs from the system */
	sysctl(mib, 2, &numCPU, &len, NULL, 0);
	
	if (numCPU < 1) 
	{
		mib[1] = HW_NCPU;
		sysctl(mib, 2, &numCPU, &len, NULL, 0);
		if (numCPU < 1) return 1;
	}
	return numCPU;
#else
	return 0;
#endif
}



