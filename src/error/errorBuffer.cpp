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
#include "strus/base/static_assert.hpp"
#include "strus/base/malloc.hpp"
#include "strus/base/platform.hpp"
#include "strus/base/stdarg.h"
#include "private/internationalization.hpp"
#include <stdarg.h>
#include <cstring>
#include <cstdio>

using namespace strus;

ProcessErrorBuffer::ProcessErrorBuffer()
	:m_hasmsg(false),m_info(0)
{
	STRUS_STATIC_ASSERT( sizeof(*this) == ObjSize);
	STRUS_STATIC_ASSERT( ObjSize % strus::platform::CacheLineSize == 0);
	m_msgbuf[ 0] = '\0';
}

void ProcessErrorBuffer::releaseInfo()
{
	if (m_info)
	{
		Info* next = m_info->next;
		while (next)
		{
			std::free( m_info);
			m_info = next;
			next = m_info->next;
		}
		std::free( m_info);
		m_info = 0;
	}
}

static int getDigit( unsigned char ch)
{
	if (ch >= '0' && ch <= '9')
	{
		return ch - '0';
	}
	else
	{
		return -1;
	}
}

void ProcessErrorBuffer::issueError( FILE* logfilehandle, int errcode, const char* msg)
{
	if (!m_hasmsg)
	{
		std::snprintf( m_msgbuf, sizeof(m_msgbuf), _TXT("[#%d] %s"), errcode, msg);
		m_hasmsg = true;
	}
	if (logfilehandle)
	{
		fprintf( logfilehandle, "%s\n", msg);
		fflush( logfilehandle);
	}
}

void ProcessErrorBuffer::issueInfo( FILE* logfilehandle, const char* format, va_list arg)
{
	va_list arg_for_calculate_msg_length;
	va_copy( arg_for_calculate_msg_length, arg);
	char buf[ 4];
	int infolen = std::vsnprintf( buf, sizeof(buf), format, arg_for_calculate_msg_length);
	if (infolen < 0)
	{
		issueError( logfilehandle, ErrorCodeSyntax, _TXT("format string error"));
	}
	else
	{
		Info* infonode = (Info*)std::malloc( sizeof(Info)+infolen);
		if (!infonode)
		{
			issueError( logfilehandle, ErrorCodeOutOfMem, _TXT("out of memory in output info"));
		}
		else
		{
			infonode->next = 0;
			std::vsnprintf( infonode->msg, infolen+1, format, arg);
			if (!m_info)
			{
				m_info = infonode;
			}
			else
			{
				Info* tail = m_info;
				for (; tail->next; tail = tail->next){}
				tail->next = infonode;
			}
			if (logfilehandle)
			{
				fprintf( logfilehandle, "%s\n", infonode->msg);
				fflush( logfilehandle);
			}
		}
	}
}

void ProcessErrorBuffer::report( int errorcode, FILE* logfilehandle, const char* format, va_list arg)
{
	if (!m_hasmsg)
	{
		char errid[ 32];
		char newmsgbuf[ MsgBufSize];
		int msglen = 0;
		int hdrlen = 0;
		if (errorcode > 0)
		{
			hdrlen = std::snprintf( errid, sizeof(errid), "[#%d] ", errorcode);
			std::memcpy( newmsgbuf, errid, hdrlen);
			errid[ hdrlen-1] = 0;
		}
		msglen = hdrlen + strus_vsnprintf( newmsgbuf+hdrlen, sizeof(newmsgbuf)-hdrlen, format, arg);
		char const* newmsg = newmsgbuf;

		char const* mptr = std::strstr( newmsgbuf+hdrlen, errid);
		if (mptr)
		{
			char const* mi = mptr + hdrlen;
			char const* pi = newmsgbuf + hdrlen;
			while (*mi && *mi != '[' && *mi != ':' && *pi == *mi) {++mi;++pi;}
			if (*mi == '[' || *mi == ':' || *mi == '\0')
			{
				msglen -= (mptr - newmsg);
				newmsg = mptr;
			}
		}
		if (logfilehandle)
		{
			fprintf( logfilehandle, "%s\n", newmsg);
			fflush( logfilehandle);
		}
		std::memcpy( m_msgbuf, newmsg, msglen);
		m_hasmsg = true;
	}
	else if (logfilehandle)
	{
		char linebuf[ MsgBufSize];
		strus_vsnprintf( linebuf, sizeof(linebuf), format, arg);
		fprintf( logfilehandle, "%s\n", linebuf);
	}
}

void ProcessErrorBuffer::explain( FILE* logfilehandle, const char* format)
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


ErrorBuffer::ErrorBuffer( FILE* logfilehandle_, std::size_t maxNofThreads_, DebugTraceInterface* dbgtrace_)
	:m_logfilehandle(logfilehandle_),m_size(0),m_slots(0),m_ar(0),m_debugtrace(dbgtrace_)
{
	if (!initMaxNofThreads( maxNofThreads_==0?DefaultMaxNofThreads:maxNofThreads_)) throw std::bad_alloc();
}

ErrorBuffer::~ErrorBuffer()
{
	clearBuffers();
	if (m_debugtrace) delete m_debugtrace;
}

int ErrorBuffer::nextErrorCode( char const*& msgitr)
{
	while (!!(msgitr = std::strstr( msgitr, "[#")))
	{
		int rt = 0;
		int dg;
		for (msgitr += 2; 0<=(dg=getDigit(*msgitr)); ++msgitr)
		{
			rt *= 10;
			rt += dg;
		}
		if (*msgitr == ']')
		{
			++msgitr;
			return rt;
		}
	}
	return -1;
}

void ErrorBuffer::removeErrorCodes( char* msg)
{
	char const* mi = msg;
	while (*mi)
	{
		if (mi[0] == '[' && mi[1] == '#')
		{
			char* fw = msg;
			*fw++ = *mi++;
			*fw++ = *mi++;
			int nm = 0;
			int dg;
			for (; 0<=(dg=getDigit(*mi)); ++nm){*fw++ = *mi++;}
			if (*mi == ']')
			{
				++mi;
			}
			else
			{
				msg = fw;
			}
		}
		else
		{
			*msg++ = *mi++;
		}
	}
	*msg = '\0';
}

void ErrorBuffer::clearBuffers()
{
	std::size_t ii=0;
	for (; ii<m_size; ++ii)
	{
		m_slots[ii].~Slot();
		m_ar[ii].~ProcessErrorBuffer();
	}
	strus::aligned_free( (void*)m_ar);
	strus::aligned_free( (void*)m_slots);
	m_ar = 0;
	m_slots = 0;
	m_size = 0;
}

bool ErrorBuffer::initMaxNofThreads( unsigned int maxNofThreads)
{
	if (maxNofThreads == 0) maxNofThreads = DefaultMaxNofThreads;
	if (m_debugtrace && !m_debugtrace->setMaxNofThreads( maxNofThreads))
	{
		return false;
	}
	void* mem_slots = strus::aligned_malloc( maxNofThreads * sizeof(Slot), strus::platform::CacheLineSize);
	void* mem_ar = strus::aligned_malloc( maxNofThreads * sizeof(ProcessErrorBuffer), strus::platform::CacheLineSize);
	if (!mem_slots || !mem_ar) goto ERROR_EXIT;

	if (m_slots || m_ar) clearBuffers();
	std::memset( mem_slots, 0, maxNofThreads * sizeof(Slot));
	std::memset( mem_ar, 0, maxNofThreads * sizeof(ProcessErrorBuffer));
	m_slots = new(mem_slots)Slot[ m_size];
	m_ar = new(mem_ar)ProcessErrorBuffer[ m_size];
	m_size = maxNofThreads;
	return true;

ERROR_EXIT:
	if (mem_slots) strus::aligned_free( mem_slots);
	if (mem_ar) strus::aligned_free( mem_ar);
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
	strus::ThreadId::Type tid = strus::ThreadId::get();
	std::size_t ti;
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
				fprintf( m_logfilehandle, _TXT("number of threads (%u) in error buffer exhausted\n"), (unsigned int)m_size);
			}
			throw strus::runtime_error( _TXT("number of threads (%u) in error buffer exhausted"), (unsigned int)m_size);
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
	strus::ThreadId::Type tid = strus::ThreadId::get();
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

void ErrorBuffer::report( int errorcode, const char* format, ...)
{
	std::size_t ti = threadidx();
	va_list ap;
	va_start(ap, format);

	m_ar[ ti].report( errorcode, m_logfilehandle, format, ap);

	va_end(ap);
}

void ErrorBuffer::info( const char* format, ...)
{
	std::size_t ti = threadidx();
	va_list ap;
	va_start( ap, format);

	m_ar[ ti].issueInfo( m_logfilehandle, format, ap);

	va_end(ap);
}

void ErrorBuffer::explain( const char* format)
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

std::vector<std::string> ErrorBuffer::fetchInfo()
{
	std::size_t ti = threadidx();
	ProcessErrorBuffer::Info const* infostruct = m_ar[ ti].info();
	try
	{
		std::vector<std::string> rt;
		for (; infostruct; infostruct = infostruct->next)
		{
			rt.push_back( infostruct->msg);
		}
		m_ar[ ti].releaseInfo();
		return rt;
	}
	catch (const std::bad_alloc&)
	{
		m_ar[ ti].issueError( m_logfilehandle, ErrorCodeOutOfMem, _TXT("out of memory fetching info"));
		return std::vector<std::string>();
	}
}

bool ErrorBuffer::hasInfo() const
{
	std::size_t ti = threadidx();
	return m_ar[ ti].info();
}

