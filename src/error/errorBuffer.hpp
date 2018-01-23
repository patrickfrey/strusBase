/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Standard implemenation of interface for reporting and catching errors
/// \file errorBuffer.hpp
#ifndef _STRUS_ERROR_BUFFER_IMPLEMENTATION_HPP_INCLUDED
#define _STRUS_ERROR_BUFFER_IMPLEMENTATION_HPP_INCLUDED
#include "strus/errorBufferInterface.hpp"
#include "strus/base/atomic.hpp"
#include "strus/base/thread.hpp"
#include <cstdio>
#include <cstdarg>

/// \brief strus toplevel namespace
namespace strus
{

/// \class ProcessErrorBuffer
/// \brief Error buffer context for one thread
class ProcessErrorBuffer
{
public:
	ProcessErrorBuffer();
	~ProcessErrorBuffer(){}

	void report( int errorcode, FILE* logfilehandle, const char* format, va_list arg);
	void explain( FILE* logfilehandle, const char* format);

	const char* fetchError()
	{
		if (!m_hasmsg) return 0;
		m_hasmsg = false;
		return m_msgbuf;
	}

	bool hasError() const
	{
		return m_hasmsg;
	}

private:
	enum {ObjSize=512};
	enum {MsgBufSize=(ObjSize-sizeof(bool))};
	char m_msgbuf[ MsgBufSize];
	bool m_hasmsg;
};


/// \class ErrorBuffer
class ErrorBuffer
	:public ErrorBufferInterface
{
public:
	enum {DefaultMaxNofThreads=32};

	ErrorBuffer( FILE* logfilehandle_, std::size_t maxNofThreads_);
	virtual ~ErrorBuffer();

	virtual void setLogFile( FILE* hnd);

	virtual bool setMaxNofThreads( unsigned int maxNofThreads);

	virtual void report( int errorcode, const char* format, ...);

	virtual void explain( const char* format);

	virtual const char* fetchError();

	virtual bool hasError() const;

	virtual void allocContext();
	virtual void releaseContext();

private:
	std::size_t threadidx() const;
	bool initMaxNofThreads( unsigned int maxNofThreads);
	void clearBuffers();

private:
	mutable FILE* m_logfilehandle;
	std::size_t m_size;
	struct Slot
	{
		typedef strus::ThreadId::Type Id;
		typedef strus::AtomicFlag Flag;

		Slot(){}
		~Slot(){}

		Id id;
		Flag flag;
	};

	Slot* m_slots;
	ProcessErrorBuffer* m_ar;
};

}//namespace
#endif

