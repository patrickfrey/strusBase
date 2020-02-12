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
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class DebugTraceInterface;

/// \class ProcessErrorBuffer
/// \brief Error buffer context for one thread
class ProcessErrorBuffer
{
public:
	ProcessErrorBuffer();
	~ProcessErrorBuffer(){}

	void report( int errorcode, FILE* logfilehandle, const char* format, va_list arg);
	void explain( FILE* logfilehandle, const char* format);

	void issueInfo( FILE* logfilehandle, const char* format, va_list arg);

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

public:
	struct Info {
		Info* next;
		char msg[ 1];
	};

	Info* info() const
	{
		return m_info;
	}

	void releaseInfo();
	void issueError( FILE* logfilehandle, int errcode, const char* msg);

private:
	enum {ObjSize=1024};
	enum {MsgBufSize=(ObjSize - 3*sizeof(Info*))};
	char m_msgbuf[ MsgBufSize];
	bool m_hasmsg;
	bool _[ sizeof(Info*)-1];
	Info* m_info;
	Info* m_info_tail;
};


/// \class ErrorBuffer
class ErrorBuffer
	:public ErrorBufferInterface
{
public:
	enum {DefaultMaxNofThreads=32};

	/// \brief Constructor
	/// \param[in] logfilehandle_ handle for logfile
	/// \param[in] maxNofThreads_ maximum number of simultaneus threads open, using the error buffer
	/// \param[in] dbgtrace_ debug trace interface (passed with ownership)
	ErrorBuffer( FILE* logfilehandle_, std::size_t maxNofThreads_, DebugTraceInterface* dbgtrace_);

	/// \brief Destructor
	virtual ~ErrorBuffer();

	virtual void setLogFile( FILE* hnd);

	virtual bool setMaxNofThreads( unsigned int maxNofThreads);

	virtual void report( int errorcode, const char* format, ...);
	virtual void explain( const char* format);

	virtual void info( const char* format, ...);

	virtual const char* fetchError();
	virtual bool hasError() const;

	virtual std::vector<std::string> fetchInfo();
	virtual bool hasInfo() const;

	virtual void allocContext();
	virtual void releaseContext();

	virtual DebugTraceInterface* debugTrace() const
	{
		return m_debugtrace;
	}

	static int nextErrorCode( char const*& msgitr);
	static void removeErrorCodes( char* msg);

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
	DebugTraceInterface* m_debugtrace;
};

}//namespace
#endif

