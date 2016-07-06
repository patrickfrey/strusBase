/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implemenation of interface for reporting and catching errors in strus for implementing an expcetion free interface
/// \file errorBuffer.cpp
#include "errorBuffer.hpp"
#include "strus/base/snprintf.h"
#include "private/internationalization.hpp"
#include "private/utils.hpp"
#include <stdarg.h>
#include <cstring>
#include <cstdio>

using namespace strus;

ProcessErrorBuffer::ProcessErrorBuffer()
	:m_hasmsg(false)
{
	STRUS_STATIC_ASSERT( sizeof(*this) == ObjSize);
	STRUS_STATIC_ASSERT( ObjSize % CACHELINE_SIZE == 0);
	m_msgbuf[ 0] = '\0';
}

void ProcessErrorBuffer::report( FILE* logfilehandle, const char* format, va_list arg) const
{
	if (!m_hasmsg)
	{
		char newmsgbuf[ MsgBufSize];
		strus_vsnprintf( newmsgbuf, sizeof(newmsgbuf), format, arg);
		if (logfilehandle)
		{
			fprintf( logfilehandle, "%s\n", newmsgbuf);
			fflush( logfilehandle);
		}
		std::memcpy( m_msgbuf, newmsgbuf, sizeof(m_msgbuf));
		m_hasmsg = true;
	}
	else if (logfilehandle)
	{
		char linebuf[ MsgBufSize];
		strus_vsnprintf( linebuf, sizeof(linebuf), format, arg);
		fprintf( logfilehandle, "%s\n", linebuf);
	}
}

void ProcessErrorBuffer::explain( FILE* logfilehandle, const char* format) const
{
	char newmsgbuf[ MsgBufSize];
	if (!m_hasmsg) m_msgbuf[0] = '\0';
	strus_snprintf( newmsgbuf, sizeof(newmsgbuf), format, m_msgbuf);
	if (logfilehandle)
	{
		fprintf( logfilehandle, "%s\n", newmsgbuf);
		fflush( logfilehandle);
	}
	std::strcpy( m_msgbuf, newmsgbuf);
	m_hasmsg = true;
}


ErrorBuffer::ErrorBuffer( FILE* logfilehandle_, std::size_t maxNofThreads_)
	:m_logfilehandle(logfilehandle_),m_size(0),m_slots(0),m_ar(0)
{
	if (!initMaxNofThreads( maxNofThreads_==0?DefaultMaxNofThreads:maxNofThreads_)) throw std::bad_alloc();
}

ErrorBuffer::~ErrorBuffer()
{
	clearBuffers();
}

void ErrorBuffer::clearBuffers()
{
	std::size_t ii=0;
	for (; ii<m_size; ++ii)
	{
		m_slots[ii].~Slot();
		m_ar[ii].~ProcessErrorBuffer();
	}
	utils::aligned_free( (void*)m_ar);
	utils::aligned_free( (void*)m_slots);
	m_ar = 0;
	m_slots = 0;
	m_size = 0;
}

bool ErrorBuffer::initMaxNofThreads( unsigned int maxNofThreads)
{
	if (maxNofThreads == 0) maxNofThreads = DefaultMaxNofThreads;

	void* mem_slots = utils::aligned_malloc( maxNofThreads * sizeof(Slot), CACHELINE_SIZE);
	void* mem_ar = utils::aligned_malloc( maxNofThreads * sizeof(ProcessErrorBuffer), CACHELINE_SIZE);
	if (!mem_slots || !mem_ar) goto ERROR_EXIT;

	if (m_slots || m_ar) clearBuffers();
	std::memset( mem_slots, 0, maxNofThreads * sizeof(Slot));
	std::memset( mem_ar, 0, maxNofThreads * sizeof(ProcessErrorBuffer));
	m_slots = new(mem_slots)Slot[ m_size];
	m_ar = new(mem_ar)ProcessErrorBuffer[ m_size];
	m_size = maxNofThreads;
	return true;

ERROR_EXIT:
	if (mem_slots) utils::aligned_free( mem_slots);
	if (mem_ar) utils::aligned_free( mem_ar);
	return false;
}

bool ErrorBuffer::setMaxNofThreads( unsigned int maxNofThreads)
{
	if (!initMaxNofThreads( maxNofThreads))
	{
		if (m_logfilehandle)
		{
			fprintf( m_logfilehandle, "%s", _TXT("out of memory initializing standard error buffer\n"));
		}
		return false;
	}
	return true;
}

void ErrorBuffer::setLogFile( FILE* hnd)
{
	m_logfilehandle = hnd;
}

std::size_t ErrorBuffer::threadidx() const
{
	utils::ThreadId::Type tid = utils::ThreadId::get();
	std::size_t ti;
	Slot xx;
	for (ti=0; ti<m_size; ++ti)
	{
		if (m_slots[ti].flag.test() && m_slots[ti].id == tid) break;
	}
	if (ti == m_size)
	{
		// ... thread has no context yet occupied. We occupy one
		for (ti=0; ti<m_size; ++ti)
		{
			if (m_slots[ti].flag.set( true)) break;
		}
		if (ti == m_size)
		{
			if (m_logfilehandle)
			{
				fprintf( m_logfilehandle, "%s", _TXT("number of threads in error buffer exhausted\n"));
			}
			throw std::logic_error( _TXT("number of threads in error buffer exhausted"));
		}
		m_slots[ti].id = tid;
	}
	return ti;
}

void ErrorBuffer::allocContext()
{
	(void)threadidx();
}

void ErrorBuffer::releaseContext()
{
	utils::ThreadId::Type tid = utils::ThreadId::get();
	std::size_t ti;
	for (ti=0; ti<m_size; ++ti)
	{
		if (m_slots[ti].flag.test() && m_slots[ti].id == tid) break;
	}
	if (ti < m_size)
	{
		m_ar[ti].fetchError();
		m_slots[ti].flag.set(false);
	}
}

void ErrorBuffer::report( const char* format, ...) const
{
	std::size_t ti = threadidx();
	va_list ap;
	va_start(ap, format);

	m_ar[ ti].report( m_logfilehandle, format, ap);

	va_end(ap);
}

void ErrorBuffer::explain( const char* format) const
{
	std::size_t ti = threadidx();
	m_ar[ ti].explain( m_logfilehandle, format);
}

const char* ErrorBuffer::fetchError()
{
	std::size_t ti = threadidx();
	return m_ar[ ti].fetchError();
}

bool ErrorBuffer::hasError() const
{
	std::size_t ti = threadidx();
	return m_ar[ ti].hasError();
}


