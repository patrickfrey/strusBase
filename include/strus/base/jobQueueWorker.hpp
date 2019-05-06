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
#include <stdexcept>
#include <new>

namespace strus {

typedef void (*JobNotifyProc)( void* context, void* data, int errcode);
typedef void (*JobHandlerProc)( void* context, void* data, JobNotifyProc notify);
typedef void (*JobHandlerTicker)( void* context);

/// \brief Job queue worker and periodic timer event ticker thread
class JobQueueWorker
{
public:
	enum {NumberOfRequestsBeforeTick=5};

	/// \brief Constructor
	/// \param[in] Period of timer ticker events in seconds
	/// \note Period equals timeout without worker jobs triggered
	/// \remark Regularity of timer events is not guaranteed, bookkeeping about real time is up to client
	explicit JobQueueWorker( int secondsPeriod_)
	{
		if (!init(secondsPeriod_)) throw std::bad_alloc();
	}
	/// \brief Destructor
	virtual ~JobQueueWorker()
	{
		clear();
	}

	/// \brief Start thread
	bool start();
	/// \brief Stop thread without finishing idle jobs in the queue
	void stop();

	/// \brief Push a new job to be executed
	/// param[in] proc procedure of the job to be executed
	/// param[in] context context data of the job to be executed
	/// param[in] data parameter data of the job to be executed
	/// param[in] notify notification function called on job completion or error
	bool pushJob( JobHandlerProc proc, void* context, void* data, JobNotifyProc notify);

	/// \brief Push a ticker procedure called periodically
	/// param[in] proc procedure of the ticker to be executed
	/// param[in] context context data of the ticker to be executed
	bool pushTicker( JobHandlerTicker proc, void* context);

private:
	void wait();
	bool init( int secondsPeriod_);
	void clear();

private:
	struct Data;
	Data* m_data;
};

} // namespace
#endif
