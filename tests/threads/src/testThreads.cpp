/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/thread.hpp"
#include "strus/base/atomic.hpp"
#include "strus/base/shared_ptr.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>

#undef STRUS_LOWLEVEL_DEBUG

class GlobalCounterWithLock
{
public:
	explicit GlobalCounterWithLock( long startvalue_)
		:m_mutex(),m_counter(startvalue_){}

	void run( int nofTimes, int increment)
	{
		int ni=0, ne=nofTimes;
		for (; ni != ne; ++ni)
		{
			strus::scoped_lock lock( m_mutex);
			m_counter += increment;
		}
	}

	void checkCounter( long expected)
	{
		if (m_counter != expected) 
		{
			char buf[ 1024];
			std::snprintf( buf, sizeof(buf), "counter value %ld in test of threads with locks not as expected (%ld)", m_counter, expected);
			throw std::runtime_error( buf);
		}
	}

private:
	strus::mutex m_mutex;
	long m_counter;
};

class GlobalCounterWithAtomic
{
public:
	explicit GlobalCounterWithAtomic( long startvalue_)
		:m_counter(startvalue_){}

	void run( int nofTimes, int increment)
	{
		int ni=0, ne=nofTimes;
		for (; ni != ne; ++ni)
		{
			m_counter.increment( increment);
		}
	}

	void checkCounter( long expected)
	{
		if (m_counter.value() != expected) 
		{
			char buf[ 1024];
			std::snprintf( buf, sizeof(buf), "counter value %ld in test of threads with atomic not as expected (%ld)", m_counter.value(), expected);
			throw std::runtime_error( buf);
		}
	}

private:
	strus::AtomicCounter<long> m_counter;
};

template <class GlobalCounter>
static void testThreads( const char* testname, int times)
{
	int ti = 0, te = times;
	for (; ti != te; ++ti)
	{
		int start = ti + 1000 + 1;
		int nofThreads = ti*10;
		std::cerr << "starting test " << testname << "." << ti << " (" << (2*nofThreads) << " threads ..." << std::endl;
		GlobalCounter obj( start);
		std::vector<strus::shared_ptr<strus::thread> > threadGroup;

		for (int si=0; si <= nofThreads; ++si)
		{
			strus::shared_ptr<strus::thread> th( new strus::thread( &GlobalCounter::run, &obj, ti * 1000, si));
			threadGroup.push_back( th);
		}
		for (int si=0; si <= nofThreads; ++si)
		{
			strus::shared_ptr<strus::thread> th( new strus::thread( &GlobalCounter::run, &obj, ti * 1000, -si));
			threadGroup.push_back( th);
		}
		std::cerr << "waiting for threads to terminate ..." << std::endl;
		std::vector<strus::shared_ptr<strus::thread> >::iterator gi = threadGroup.begin(), ge = threadGroup.end();
		for (; gi != ge; ++gi)
		{
			(*gi)->join();
		}
		std::cerr << "done" << std::endl;
		obj.checkCounter( start);
	}
}


#define NOF_TESTS 1000

int main( int, const char**)
{
	try
	{
		testThreads<GlobalCounterWithLock>( "with-lock", 10);
		testThreads<GlobalCounterWithAtomic>( "with-atomic", 10);
		std::cerr << "OK" << std::endl;
		return 0;
	}
	catch (const std::bad_alloc& err)
	{
		std::cerr << "ERROR " << err.what() << std::endl;
	}
	catch (const std::exception& err)
	{
		std::cerr << "ERROR " << err.what() << std::endl;
	}
	return -1;
}


