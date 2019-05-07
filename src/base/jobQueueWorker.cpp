/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Periodic timer event
#include "strus/base/dll_tags.hpp"
#include "strus/base/jobQueueWorker.hpp"
#include "strus/base/atomic.hpp"
#include <queue>

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

struct JobQueueWorker::Data
{
	struct Ticker
	{
		Ticker() :proc(0),context(0){}
		Ticker( JobHandlerTicker proc_, void* context_) :proc(proc_),context(context_){}
		Ticker( const Ticker& o) :proc(o.proc),context(o.context){}
	
		JobHandlerTicker proc;
		void* context;
	};
	struct Element
	{
		Element() :proc(0),notify(0),deleter(0),context(0){}
		Element( JobHandlerProc proc_, void* context_, JobNotifyProc notify_, JobDeleterProc deleter_) :proc(proc_),notify(notify_),deleter(deleter_),context(context_){}
		Element( const Element& o) :proc(o.proc),notify(o.notify),deleter(o.deleter),context(o.context){}

		JobHandlerProc proc;
		JobNotifyProc notify;
		JobDeleterProc deleter;
		void* context;
	};

	explicit Data( int secondsPeriod_)
		:thread(0)
		,secondsPeriod(secondsPeriod_)
		,requestCount(0)
		,numberOfRequestsBeforeTick(secondsPeriod_*NumberOfRequestsBeforeTick)
		,terminate(false)
	{}

	bool pushJob( JobHandlerProc proc, void* context, JobNotifyProc notify, JobDeleterProc deleter)
	{
		try
		{
			intent.increment();
			pte::unique_lock<pte::mutex> lock( qe_mutex);
			queue.push( Element( proc, context, notify, deleter));
			cv.notify_one();
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	bool pushTicker( JobHandlerTicker proc, void* context)
	{
		try
		{
			pte::unique_lock<pte::mutex> lock( tc_mutex);
			tickers.push_back( Ticker( proc, context));
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	void tick()
	{
		pte::unique_lock<pte::mutex> lock( tc_mutex);
		std::vector<Ticker>::const_iterator ti = tickers.begin(), te = tickers.end();
		for (; ti != te; ++ti)
		{
			ti->proc( ti->context);
		}
	}

	bool fetch( Element& job)
	{
		try
		{
			pte::unique_lock<pte::mutex> lock( qe_mutex);
			if (queue.empty()) return false;
			job = queue.front();
			queue.pop();
			intent.decrement();
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	bool forcedTick()
	{
		if (++requestCount == numberOfRequestsBeforeTick)
		{
			requestCount = 0;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool terminated() const
	{
		return terminate.test();
	}


	bool start( pte::thread* thread_)
	{
		if (thread) return false;
		terminate.set( false);
		thread = thread_;
		return true;
	}

	bool stopped()
	{
		return thread == NULL;
	}

	void stop()
	{
		if (thread)
		{
			intent.increment();
			terminate.set( true);
			cv.notify_all();
			thread->join();
			delete thread;
			thread = 0;
		}
	}

	inline pte::cv_status wait()
	{
		pte::unique_lock<pte::mutex> lock( cv_mutex);
		if (intent.value())
		{
			return pte::cv_status::no_timeout;
		}
		else
		{
			return cv.wait_for( lock, pte::chrono::seconds( secondsPeriod));
		}
	}

private:
	pte::condition_variable cv;
	pte::mutex cv_mutex;
	pte::mutex qe_mutex;
	pte::mutex tc_mutex;
	pte::thread* thread;
	std::queue<Element> queue;
	std::vector<Ticker> tickers;
	int secondsPeriod;
	int requestCount;
	int numberOfRequestsBeforeTick;
	AtomicFlag terminate;
	AtomicCounter<int> intent;
};


DLL_PUBLIC bool JobQueueWorker::init( int secondsPeriod_)
{
	try
	{
		m_data = new Data( secondsPeriod_);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

DLL_PUBLIC void JobQueueWorker::clear()
{
	stop();
	delete m_data;
}

DLL_PUBLIC void JobQueueWorker::wait()
{
	for (;;)
	{
		pte::cv_status status = m_data->wait();
		if (m_data->terminated())
		{
			return;
		}
		else if (status == pte::cv_status::no_timeout)
		{
			JobQueueWorker::Data::Element job;
			while (m_data->fetch( job))
			{
				job.proc( job.context, job.notify, job.deleter);
				if (m_data->forcedTick()) m_data->tick();
			}
		}
		else
		{
			//... timeout, do action
			m_data->tick();
		}
	}
}

DLL_PUBLIC bool JobQueueWorker::start()
{
	try
	{
		if (!m_data->stopped()) return false;
		return m_data->start( new pte::thread( &JobQueueWorker::wait, this));
	}
	catch (...)
	{
		return false;
	}
}

DLL_PUBLIC void JobQueueWorker::stop()
{
	m_data->stop();
}

DLL_PUBLIC bool JobQueueWorker::pushJob( JobHandlerProc proc, void* context, JobNotifyProc notify, JobDeleterProc deleter)
{
	return m_data->pushJob( proc, context, notify, deleter);
}

DLL_PUBLIC bool JobQueueWorker::pushTicker( JobHandlerTicker proc, void* context)
{
	return m_data->pushTicker( proc, context);
}

