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
#include "strus/base/bitset.hpp"
#include <queue>
#include <ctime>
#include <unistd.h>
#include <fcntl.h>

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
	struct FdSet
	{
		fd_set read;
		fd_set write;
		fd_set excep;
		struct timeval timeout;

		FdSet( int timeout_secs)
		{
			FD_ZERO( &read);
			FD_ZERO( &write);
			FD_ZERO( &excep);
			timeout.tv_sec = timeout_secs;
			timeout.tv_usec = 0;
		}
		FdSet( const FdSet& o)
		{
			std::memcpy( this, &o, sizeof(*this));
		}
		FdSet& operator = ( const FdSet& o)
		{
			std::memcpy( this, &o, sizeof(*this));
			return *this;
		}
	};

	struct SelectJobStruct
	{
		JobHandlerProc proc;
		JobDeleterProc deleter;
		void* context;
		int refcnt;

		SelectJobStruct()
			:proc(0),deleter(0),context(0),refcnt(0){}
		SelectJobStruct( JobHandlerProc proc_, JobDeleterProc deleter_, void* context_)
			:proc(proc_),deleter(deleter_),context(context_),refcnt(1){}
		SelectJobStruct( const SelectJobStruct& o)
			:proc(o.proc),deleter(o.deleter),context(o.context),refcnt(o.refcnt){}
		SelectJobStruct& operator=( const SelectJobStruct& o)
			{proc=o.proc; deleter=o.deleter; context=o.context; refcnt=o.refcnt; return *this;}
	};

	struct SelectFdData
	{
		FdSet fdset;
		int nofd;
		int pipfd[2];

		unsigned char jobMap_read[ FD_SETSIZE];
		unsigned char jobMap_write[ FD_SETSIZE];
		unsigned char jobMap_excep[ FD_SETSIZE];
		enum {MaxNofJobs=256};
		SelectJobStruct jobs[MaxNofJobs];
		int nofJobs;

		SelectFdData( int timeout_secs)
			:fdset( timeout_secs),nofJobs(0)
		{
			pipfd[0] = 0;
			pipfd[1] = 0;
			if (pipe(pipfd) == -1) throw std::bad_alloc();
			nofd = pipfd[0]+1;
			FD_SET( pipfd[0], &fdset.read);

			std::memset( jobMap_read, 0, sizeof(jobMap_read));
			std::memset( jobMap_write, 0, sizeof(jobMap_read));
			std::memset( jobMap_excep, 0, sizeof(jobMap_read));

			int flags = fcntl(pipfd[0], F_GETFL);
			if (flags == -1) goto ERROR;
			flags |= O_NONBLOCK;
			if (fcntl(pipfd[0], F_SETFL, flags) == -1) goto ERROR;
			flags = fcntl(pipfd[1], F_GETFL);
			if (flags == -1) goto ERROR;
			flags |= O_NONBLOCK;
			if (fcntl(pipfd[1], F_SETFL, flags) == -1) goto ERROR;
			return;
		ERROR:
			close( pipfd[0]);
			close( pipfd[1]);
			throw std::bad_alloc();
		}
		~SelectFdData()
		{
			close( pipfd[0]);
			close( pipfd[1]);
		}

		void notify()
		{
			write( pipfd[1], "x", 1);
		}

		unsigned char allocJobHandle( JobHandlerProc proc, JobDeleterProc deleter, void* context)
		{
			int ji = 0;
			for (; ji < nofJobs; ++ji)
			{
				if (jobs[ ji].proc == proc && jobs[ ji].context == context && jobs[ ji].deleter == deleter) break;
			}
			if (ji == nofJobs)
			{
				for (ji=0; ji < nofJobs; ++ji)
				{
					if (jobs[ ji].refcnt == 0) break;
				}
				if (ji == nofJobs)
				{
					if (nofJobs+1 == MaxNofJobs) return 0;
					++nofJobs;
				}
				jobs[ ji].proc = proc;
				jobs[ ji].deleter = deleter;
				jobs[ ji].context = context;
				jobs[ ji].refcnt = 0;
			}
			return (unsigned char)(unsigned int)(ji + 1);
		}

		void setJobFlags( unsigned char* jobmap, fd_set* fs, int fh, unsigned char jobhnd)
		{
			FD_SET( fh, fs); 
			jobmap[ fh] = jobhnd;
			++jobs[ jobhnd-1].refcnt;
			if (fh >= nofd) nofd = fh+1;
		}

		void clrJobFlags( unsigned char* jobmap, fd_set* fs, int fh)
		{
			int jobhnd = (unsigned int)jobmap[ fh];
			if (jobhnd)
			{
				FD_CLR( fh, fs);
				if (--jobs[ jobhnd-1].refcnt == 0)
				{
					SelectJobStruct& job = jobs[ jobhnd-1];
					if (job.deleter && job.context)
					{
						job.deleter( job.context);
					}
					job.proc = 0;
					job.deleter = 0;
					job.context = 0;
					job.refcnt = 0;
				}
				jobmap[ fh] = 0;
			}
		}

		bool defineListener( JobHandlerProc proc, JobDeleterProc deleter, void* context, int fh, int fdTypeMask)
		{
			if (fh >= FD_SETSIZE) return false;

			// [1] Find free job handle:
			unsigned char jobhnd = allocJobHandle( proc, deleter, context);
			if (!jobhnd) return false;

			// [2] Initialize listener:
			if ((fdTypeMask & FdRead) == FdRead) setJobFlags( jobMap_read, &fdset.read, fh, jobhnd);
			if ((fdTypeMask & FdWrite) == FdWrite) setJobFlags( jobMap_write, &fdset.write, fh, jobhnd);
			if ((fdTypeMask & FdExcept) == FdExcept) setJobFlags( jobMap_excep, &fdset.excep, fh, jobhnd);

			return true;
		}

		void releaseNumberOfFileHandles( int fh)
		{
			if (fh == nofd-1)
			{
				while (fh >= 0 && !jobMap_read[ fh] && !jobMap_write[ fh] && !jobMap_excep[ fh])
				{
					--fh;
				}
				nofd = fh+1;
				if (nofd < (pipfd[0]+1)) nofd = (pipfd[0]+1);
			}
		}

		bool deleteListener( int fh, int fdTypeMask)
		{
			if (fh >= FD_SETSIZE) return false;

			if (!fdTypeMask)
			{
				clrJobFlags( jobMap_read, &fdset.read, fh);
				clrJobFlags( jobMap_write, &fdset.write, fh);
				clrJobFlags( jobMap_excep, &fdset.excep, fh);
			}
			else
			{
				if ((fdTypeMask & FdRead) == FdRead) clrJobFlags( jobMap_read, &fdset.read, fh);
				if ((fdTypeMask & FdRead) == FdWrite) clrJobFlags( jobMap_write, &fdset.write, fh);
				if ((fdTypeMask & FdRead) == FdExcept) clrJobFlags( jobMap_excep, &fdset.excep, fh);
			}
			releaseNumberOfFileHandles( fh);
			return true;
		}

		bool collectHnd( bitset<MaxNofJobs>& hndset, unsigned char* jobmap, fd_set* fs)
		{
			int fh = 0;
			for (; fh < nofd; ++fh)
			{
				if (FD_ISSET( fh, fs))
				{
					unsigned char hnd = jobmap[ fh];
					if (hnd) hndset.set( hnd, true);
				}
			}
			return true;
		}

		bool wait()
		{
			bool gotSignal = false;
			bitset<MaxNofJobs> hndset;

			int ready;
			FdSet fs = fdset;
			while ((ready = ::select( nofd, &fs.read, &fs.write, &fs.excep, &fs.timeout)) == -1 && errno == EINTR)
			{
				continue;
			}
			if (ready == -1) return false;
			if (FD_ISSET( pipfd[0], &fs.read))
			{
				for (;;)
				{
					char ch;
					if (::read(pipfd[0], &ch, 1) == -1) {
						if (errno == EAGAIN) break;
					}
				}
				gotSignal = true;
			}
			if (!collectHnd( hndset, jobMap_read, &fs.read)) return false;
			if (!collectHnd( hndset, jobMap_write, &fs.write)) return false;
			if (!collectHnd( hndset, jobMap_excep, &fs.excep)) return false;

			int hnd = hndset.next(-1);
			gotSignal |= (hnd >= 0);
			for (; hnd >= 0; hnd = hndset.next( hnd))
			{
				jobs[ hnd].proc( jobs[ hnd].context);
			}
			return gotSignal;
		}
	};

	struct Ticker
	{
		Ticker() :proc(0),context(0){}
		Ticker( JobHandlerProc proc_, void* context_) :proc(proc_),context(context_){}
		Ticker( const Ticker& o) :proc(o.proc),context(o.context){}
	
		JobHandlerProc proc;
		void* context;
	};

	struct Job
	{
		Job() :proc(0),deleter(0),context(0),fh(0),fdTypeMask(0){}
		Job( JobHandlerProc proc_, void* context_, JobDeleterProc deleter_) :proc(proc_),deleter(deleter_),context(context_),fh(0),fdTypeMask(0){}
		Job( JobHandlerProc proc_, void* context_, JobDeleterProc deleter_, int fh_, int fdTypeMask_) :proc(proc_),deleter(deleter_),context(context_),fh(fh_),fdTypeMask(fdTypeMask_){}
		Job( const Job& o) :proc(o.proc),deleter(o.deleter),context(o.context),fh(o.fh),fdTypeMask(o.fdTypeMask){}

		bool isListener()
		{
			return !!fdTypeMask;
		}

		JobHandlerProc proc;
		JobDeleterProc deleter;
		void* context;
		int fh;
		int fdTypeMask;
	};

	explicit Data( int secondsPeriod_, bool useFdSelect)
		:thread(0)
		,secondsPeriod(secondsPeriod_)
		,requestCount(0)
		,numberOfRequestsBeforeTick(secondsPeriod_*NumberOfRequestsBeforeTick)
		,terminate(false)
		,selectFdData( useFdSelect ? new SelectFdData( secondsPeriod_) : 0)
	{}
	~Data()
	{
		if (selectFdData) delete selectFdData;
	}

	void notify()
	{
		if (selectFdData)
		{
			selectFdData->notify();
		}
		else
		{
			cv.notify_all();
		}
	}

	bool pushJob( JobHandlerProc proc, void* context, JobDeleterProc deleter)
	{
		try
		{
			intent.increment();
			pte::unique_lock<pte::mutex> lock( qe_mutex);
			queue.push( Job( proc, context, deleter));
			notify();
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	bool pushTicker( JobHandlerProc proc, void* context)
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

	bool pushListener( JobHandlerProc proc, void* context, JobDeleterProc deleter, int fh, int fdTypeMask)
	{
		try
		{
			intent.increment();
			pte::unique_lock<pte::mutex> lock( qe_mutex);
			queue.push( Job( proc, context, deleter, fh, fdTypeMask));
			notify();
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

	bool fetch( Job& job)
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
			notify();
			thread->join();
			Job job;
			while (fetch( job))
			{
				if (job.deleter) job.deleter( job.context);
			}
			delete thread;
			thread = 0;
		}
	}

	inline pte::cv_status wait()
	{
		if (intent.value())
		{
			return pte::cv_status::no_timeout; 
		}
		else if (selectFdData)
		{
			return selectFdData->wait()
				? pte::cv_status::no_timeout
				: pte::cv_status::timeout;
		}
		else
		{
			pte::unique_lock<pte::mutex> lock( cv_mutex);
			return cv.wait_for( lock, pte::chrono::seconds( secondsPeriod));
		}
	}

	bool execJob( Job& job)
	{
		if (job.isListener())
		{
			if (selectFdData)
			{
				if (job.proc)
				{
					selectFdData->defineListener( job.proc, job.deleter, job.context, job.fh, job.fdTypeMask);
				}
				else
				{
					selectFdData->deleteListener( job.fh, job.fdTypeMask);
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			job.proc( job.context);
		}
		if (forcedTick()) tick();
		return true;
	}

private:
	pte::condition_variable cv;
	pte::mutex cv_mutex;
	pte::mutex qe_mutex;
	pte::mutex tc_mutex;
	pte::thread* thread;
	std::queue<Job> queue;
	std::vector<Ticker> tickers;
	int secondsPeriod;
	int requestCount;
	int numberOfRequestsBeforeTick;
	AtomicFlag terminate;
	AtomicCounter<int> intent;
	SelectFdData* selectFdData;
};


DLL_PUBLIC bool JobQueueWorker::init( int secondsPeriod_, bool useFdSelect)
{
	try
	{
		m_data = new Data( secondsPeriod_, useFdSelect);
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
			JobQueueWorker::Data::Job job;
			while (m_data->fetch( job))
			{
				(void)m_data->execJob( job);
			}
		}
		else
		{
			//... timeout, do tick
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

DLL_PUBLIC bool JobQueueWorker::pushJob( JobHandlerProc proc, void* context, JobDeleterProc deleter)
{
	return m_data->pushJob( proc, context, deleter);
}

DLL_PUBLIC bool JobQueueWorker::pushTicker( JobHandlerProc proc, void* context)
{
	return m_data->pushTicker( proc, context);
}

DLL_PUBLIC bool JobQueueWorker::pushListener( JobHandlerProc proc, void* context, JobDeleterProc deleter, int fh, int fdTypeMask)
{
	return m_data->pushListener( proc, context, deleter, fh, fdTypeMask);
}


