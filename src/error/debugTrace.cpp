/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Standard implemenation of interface for reporting and catching errors
/// \file debugTrace.cpp
#include "debugTrace.hpp"
#include "strus/base/string_format.hpp"
#include "strus/base/malloc.hpp"
#include "strus/base/static_assert.hpp"
#include "strus/base/platform.hpp"
#include <cstring>
#include <cstdlib>

using namespace strus;

ProcessDebugTrace::ProcessDebugTrace()
	:m_tid( strus::ThreadId::get()),m_hasError(false),m_stk(),m_msgar(),m_enablelistsize(0)
{
	std::memset( m_enablelist, 0, sizeof(m_enablelist));
}

ProcessDebugTrace::~ProcessDebugTrace()
{
	for (int ei=0; ei<m_enablelistsize; ++ei)
	{
		if (m_enablelist[ei])
		{
			std::free( m_enablelist[ei]);
		}
	}
}

bool ProcessDebugTrace::checkThreadId()
{
	if (m_tid != strus::ThreadId::get())
	{
		m_hasError.set( true);
		return false;
	}
	return true;
}

void ProcessDebugTrace::open( const char* component, const char* type, const std::string& name)
{
	if (checkThreadId()) try
	{
		m_stk.push_back( m_msgar.size());
		m_msgar.push_back( DebugTraceMessage( DebugTraceMessage::Open, component, type, name));
	}
	catch (const std::bad_alloc&)
	{
		m_hasError.set( true);
	}
}

void ProcessDebugTrace::close( const char* component)
{
	if (checkThreadId()) try
	{
		int idx = m_stk.back();
		if (idx >= 0 && idx < (int)m_msgar.size())
		{
			const DebugTraceMessage& openmsg = m_msgar[ idx];
			if (0==std::strcmp( component, openmsg.component()))
			{
				m_msgar.push_back( DebugTraceMessage( DebugTraceMessage::Close, openmsg.component(), openmsg.id(), openmsg.content()));
				m_stk.pop_back();
			}
			else
			{
				m_hasError.set( true);
			}
		}
		else
		{
			m_hasError.set( true);
		}
	}
	catch (const std::bad_alloc&)
	{
		m_hasError.set( true);
	}
}

void ProcessDebugTrace::event( const char* component, const char* eventid, const char* format, va_list arg)
{
	if (checkThreadId()) try
	{
		m_msgar.push_back( DebugTraceMessage( DebugTraceMessage::Event, component, eventid, strus::string_format( format, arg)));
	}
	catch (const std::bad_alloc&)
	{
		m_hasError.set( true);
	}
}

bool ProcessDebugTrace::enable( const std::string& component)
{
	int ei = 0;
	for (; ei < m_enablelistsize && m_enablelist[ei]; ++ei){}
	if (ei < m_enablelistsize || m_enablelistsize < SizeOfEnableList)
	{
		char* cc = (char*)std::malloc( component.size()+1);
		if (!cc)
		{
			m_hasError.set( true);
			return false;
		}
		std::memcpy( cc, component.c_str(), component.size()+1);
		m_enablelist[ ei] = cc;

		if (ei == m_enablelistsize) ++m_enablelistsize;
		return true;
	}
	else
	{
		m_hasError.set( true);
		return false;
	}
}

void ProcessDebugTrace::disable( const std::string& component)
{
	int ei = 0;
	for (; ei < m_enablelistsize; ++ei) if (m_enablelist[ei] && 0==std::strcmp( component.c_str(), m_enablelist[ei])) break;
	int ee = ei+1;
	for (; ee < m_enablelistsize; ++ee) if (m_enablelist[ee]) break;
	if (ei < m_enablelistsize)
	{
		std::free( m_enablelist[ei]);
		m_enablelist[ei] = NULL;
	}
	if (ee == m_enablelistsize) m_enablelistsize = ei;
}

bool ProcessDebugTrace::isenabled( const char* component) const
{
	int ei = 0;
	for (; ei < m_enablelistsize; ++ei) if (m_enablelist[ei] && 0==std::strcmp( component, m_enablelist[ei])) break;
	return (ei < m_enablelistsize);
}

std::vector<DebugTraceMessage> ProcessDebugTrace::fetchMessages()
{
	std::vector<DebugTraceMessage> rt = m_msgar;
	m_msgar.clear();
	m_hasError.set( false);
	return rt;
}

bool ProcessDebugTrace::hasError() const
{
	return m_hasError.test();
}


void DebugTraceContext::open( const char* type, const std::string& name)
{
	m_ref->open( m_component, type, name);
}

void DebugTraceContext::close()
{
	m_ref->close( m_component);
}

void DebugTraceContext::event( const char* eventid, const char* format, ...)
{
	va_list args;
	va_start( args, format);
	m_ref->event( m_component, eventid, format, args);
	va_end(args);
}

DebugTrace::DebugTrace( std::size_t maxNofThreads_)
	:m_size(0),m_slots(0),m_ar(0)
{
	if (!initMaxNofThreads( maxNofThreads_==0?DefaultMaxNofThreads:maxNofThreads_)) throw std::bad_alloc();
}

DebugTrace::~DebugTrace()
{
	clearBuffers();
}

typedef strus::shared_ptr<ProcessDebugTrace> ProcessDebugTraceRef;

void DebugTrace::clearBuffers()
{
	std::size_t ii=0;
	for (; ii<m_size; ++ii)
	{
		m_slots[ii].~Slot();
		m_ar[ii].~ProcessDebugTraceRef();
	}
	strus::aligned_free( (void*)m_ar);
	strus::aligned_free( (void*)m_slots);
	m_ar = 0;
	m_slots = 0;
	m_size = 0;
}

bool DebugTrace::initMaxNofThreads( unsigned int maxNofThreads)
{
	if (maxNofThreads == 0) maxNofThreads = DefaultMaxNofThreads;
	if (m_size >= maxNofThreads) return true;

	Slot* mem_slots = (Slot*)strus::aligned_malloc( maxNofThreads * sizeof(Slot), strus::platform::CacheLineSize);
	ProcessDebugTraceRef* mem_ar = (ProcessDebugTraceRef*)strus::aligned_malloc( maxNofThreads * sizeof(ProcessDebugTraceRef), strus::platform::CacheLineSize);
	if (!mem_slots || !mem_ar) goto ERROR_EXIT;

	std::memset( mem_ar, 0, maxNofThreads * sizeof(ProcessDebugTraceRef));
	std::memcpy( mem_ar, m_ar, m_size * sizeof(ProcessDebugTraceRef));
	std::memset( mem_slots, 0, maxNofThreads * sizeof(Slot));
	std::memcpy( mem_slots, m_slots, m_size * sizeof(Slot));
	new(mem_slots + m_size)Slot[ maxNofThreads - m_size];
	new(mem_ar + m_size)ProcessDebugTraceRef[ m_size];
	m_slots = mem_slots;
	m_ar = mem_ar;
	m_size = maxNofThreads;
	return true;

ERROR_EXIT:
	if (mem_slots) strus::aligned_free( mem_slots);
	if (mem_ar) strus::aligned_free( mem_ar);
	return false;
}

bool DebugTrace::setMaxNofThreads( unsigned int maxNofThreads)
{
	return initMaxNofThreads( maxNofThreads);
}

int DebugTrace::threadidx() const
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
			return -1;
		}
		m_slots[ti].id = tid;
		m_ar[ti].reset( new (std::nothrow) ProcessDebugTrace());
		if (!m_ar[ti].get()) return -1;
	}
	return ti;
}

void DebugTrace::allocContext()
{
	(void)threadidx();
}

void DebugTrace::releaseContext()
{
	strus::ThreadId::Type tid = strus::ThreadId::get();
	std::size_t ti;
	for (ti=0; ti<m_size; ++ti)
	{
		if (m_slots[ti].flag.test() && m_slots[ti].id == tid) break;
	}
	if (ti < m_size)
	{
		m_ar[ti].reset();
		m_slots[ti].flag.set(false);
	}
}

bool DebugTrace::enable( const std::string& component)
{
	int ti = threadidx();
	if (ti < 0) return false;
	return m_ar[ti]->enable( component);
}

void DebugTrace::disable( const std::string& component)
{
	int ti = threadidx();
	if (ti < 0) return;
	m_ar[ti]->disable( component);
}

bool DebugTrace::isenabled( const char* component) const
{
	int ti = threadidx();
	if (ti < 0) return false;
	strus::shared_ptr<ProcessDebugTrace> ref = m_ar[ ti];
	return ref->isenabled( component);
}

DebugTraceContextInterface* DebugTrace::createTraceContext( const char* component) const
{
	try
	{
		int ti = threadidx();
		if (ti < 0) return NULL;
		strus::shared_ptr<ProcessDebugTrace> ref = m_ar[ ti];
		if (ref->isenabled( component))
		{
			return new DebugTraceContext( component, ref);
		}
	}
	catch (const std::bad_alloc&)
	{}
	return NULL;
}

std::vector<DebugTraceMessage> DebugTrace::fetchMessages() const
{
	try
	{
		int ti = threadidx();
		if (ti < 0) return std::vector<DebugTraceMessage>();

		strus::shared_ptr<ProcessDebugTrace> ref = m_ar[ ti];
		return ref->fetchMessages();
	}
	catch (const std::bad_alloc&)
	{
		return std::vector<DebugTraceMessage>();
	}
}

bool DebugTrace::hasError() const
{
	try
	{
		int ti = threadidx();
		if (ti < 0) return true;

		strus::shared_ptr<ProcessDebugTrace> ref = m_ar[ ti];
		return ref->hasError();
	}
	catch (const std::bad_alloc&)
	{
		return true;
	}
}

