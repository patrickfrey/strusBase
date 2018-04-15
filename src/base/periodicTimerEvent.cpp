/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Periodic timer event
#include "strus/base/dll_tags.hpp"
#include "strus/base/periodicTimerEvent.hpp"

/// PF:HACK: Bad solution, need probing of atomic as C++ feature as for regex
#if defined __GNUC__
#define STRUS_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#undef STRUS_USE_STL
#if __cplusplus >= 201103L
#if defined __clang__
#define STRUS_USE_STL
#elif defined __GNUC__
#if STRUS_GCC_VERSION >= 40900
#define STRUS_USE_STL
#endif // STRUS_GCC_VERSION
#endif // __clang__
#endif // __cplusplus

#ifdef STRUS_USE_STL
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
namespace pte = std;
#else
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/chrono.hpp>
namespace pte = boost;
#endif

using namespace strus;

struct PeriodicTimerEvent::Data
{
	Data(){ thread = 0; }

	pte::condition_variable cv;
	pte::mutex cv_mutex;
	pte::thread* thread;
};

DLL_PUBLIC bool PeriodicTimerEvent::init()
{
	try
	{
		m_data = new Data();
		return true;
	}
	catch (...)
	{
		return false;
	}
}

DLL_PUBLIC void PeriodicTimerEvent::clear()
{
	stop();
	delete m_data;
}

DLL_PUBLIC void PeriodicTimerEvent::wait()
{
	for (;;)
	{
		pte::unique_lock<pte::mutex> lock( m_data->cv_mutex);
		pte::cv_status status = m_data->cv.wait_for( lock, pte::chrono::seconds(m_secondsPeriod));
		if (status == pte::cv_status::no_timeout)
		{
			//... signaled only by stop, so we terminate the thread
			return;
		}
		else
		{
			//... timeout, do action
			tick();
		}
	}
}

DLL_PUBLIC bool PeriodicTimerEvent::start()
{
	try
	{
		if (m_data->thread) stop();
		m_data->thread = new pte::thread( &PeriodicTimerEvent::wait, this);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

DLL_PUBLIC void PeriodicTimerEvent::stop()
{
	if (m_data->thread)
	{
		m_data->cv.notify_all();
		m_data->thread->join();
		delete m_data->thread;
		m_data->thread = 0;
	}
}



