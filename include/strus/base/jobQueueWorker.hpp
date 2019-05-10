/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Worker thread for asynchronous background job processing
#ifndef _STRUS_JOB_QUEUE_WORKER_HPP_INCLUDED
#define _STRUS_JOB_QUEUE_WORKER_HPP_INCLUDED
#include "strus/base/filehandle.hpp"
#include <stdexcept>
#include <new>

namespace strus {

typedef void (*JobDeleterProc)( void* context);
typedef void (*JobHandlerProc)( void* context);

/// \brief Job queue worker and periodic timer event ticker thread
class JobQueueWorker
{
public:
	enum {NumberOfRequestsBeforeTick=5};

	/// \brief Constructor
	/// \param[in] Period of timer ticker events in seconds
	/// \param[in] useFdSelect true, wait on file descriptor events (select), false wait on condition variables
	/// \note Period equals timeout without worker jobs triggered
	/// \remark Regularity of timer events is not guaranteed, bookkeeping about real time is up to client
	JobQueueWorker( int secondsPeriod_, bool useFdSelect)
	{
		if (!init(secondsPeriod_,useFdSelect)) throw std::bad_alloc();
	}
	/// \brief Destructor
	virtual ~JobQueueWorker()
	{
		clear();
	}

	/// \brief Start thread
	/// \return true on success, false on error
	bool start();
	/// \brief Stop thread without finishing idle jobs in the queue
	void stop();

	/// \brief Push a new job to be executed
	/// \param[in] proc procedure of the job to be executed
	/// \param[in] context context data of the job to be executed
	/// \param[in] deleter destructor function of the context
	/// \return true on success, false on error
	bool pushJob( JobHandlerProc proc, void* context, JobDeleterProc deleter);

	/// \brief Push a ticker procedure called periodically
	/// \param[in] proc procedure of the ticker to be executed
	/// \param[in] context context data of the ticker to be executed
	/// \return true on success, false on error
	bool pushTicker( JobHandlerProc proc, void* context);

	/// \brief Types of events to listen in the select mode (with constructor parameter useFdSelect set to true)
	enum FdType {FdRead=1,FdWrite=2,FdExcept=4};

	/// \brief Push a listener for a filehandle event 
	/// \note Only available in the select mode (with constructor parameter useFdSelect set to true)
	/// \param[in] proc procedure to be executed when the event occurrs, NULL if the listener should be removed
	/// \param[in] context context data of the procedure to be executed, NULL if the listener should be removed
	/// \param[in] deleter destructor function of the context
	/// \param[in] fh file handle
	/// \param[in] fdTypeMask mask of events to listen for
	/// \note Event handler procedure call duplicates (proc,context)-pairs may be eliminated, e.g. called only once
	/// \return true on success, false on error
	bool pushListener( JobHandlerProc proc, void* context, JobDeleterProc deleter, const FileHandle& fh, int fdTypeMask);

private:
	void wait();
	bool init( int secondsPeriod_, bool useFdSelect);
	void clear();

private:
	struct Data;
	Data* m_data;
};

} // namespace
#endif
