/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Wrapper to structures needed for multithreading
#ifndef _STRUS_THREAD_HPP_INCLUDED
#define _STRUS_THREAD_HPP_INCLUDED

#undef STRUS_USE_STD_THREAD
#if __cplusplus >= 201103L
#if defined __clang__
#define STRUS_USE_STD_THREAD
#elif defined __GNUC__
#if GCC_VERSION >= 40900
#define STRUS_USE_STD_THREAD
#endif // GCC_VERSION
#endif // __clang__
#endif // __cplusplus

#define STRUS_CACHELINE_SIZE 64
//... TODO: Probing of this constant

#ifdef STRUS_USE_STD_THREAD
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>

namespace strus {

typedef std::mutex mutex;
typedef std::recursive_mutex recursive_mutex;
typedef std::unique_lock<std::mutex> unique_lock;
typedef std::unique_lock<std::recursive_mutex> recursive_unique_lock;
typedef std::lock_guard<std::mutex> scoped_lock;
typedef std::condition_variable condition_variable;
typedef std::thread thread;

class thread_group
	:public std::vector<std::thread*>
{
	thread_group(){}
	~thread_group()
	{
		std::vector<std::thread*>::const_iterator ti = begin(), te = end();
		for (; ti != te; ++ti) delete *ti;
	}
	void add_thread( std::thread* thrd)
	{
		push_back( thrd);
	}
	void join_all()
	{
		std::vector<std::thread*>::iterator ti = begin(), te = end();
		for (; ti != te; ++ti) (*ti)->join();
	}
};

class ThreadId
{
public:
	typedef std::thread::id Type;
	static Type get()
	{
		return std::this_thread::get_id();
	}
};

}//namespace

#else //STRUS_USE_STD_THREAD
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition_variable.hpp> 
#include <boost/thread.hpp>

namespace strus {

typedef boost::mutex mutex;
typedef boost::recursive_mutex recursive_mutex;
typedef boost::mutex::scoped_lock scoped_lock;
typedef boost::unique_lock<boost::mutex> unique_lock;
typedef boost::unique_lock<boost::recursive_mutex> recursive_unique_lock;
typedef boost::condition_variable condition_variable;
typedef boost::thread thread;

class ThreadId
{
public:
	typedef boost::thread::id Type;
	static Type get()
	{
		return boost::this_thread::get_id();
	}
};

}//namespace

#endif //STRUS_USE_STD_THREAD

#endif

