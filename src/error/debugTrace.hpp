/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Standard implemenation of interface for reporting and catching errors
/// \file debugTrace.hpp
#ifndef _STRUS_DEBUG_TRACE_IMPLEMENTATION_HPP_INCLUDED
#define _STRUS_DEBUG_TRACE_IMPLEMENTATION_HPP_INCLUDED
#include "strus/debugTraceInterface.hpp"
#include "strus/base/atomic.hpp"
#include "strus/base/thread.hpp"
#include "strus/base/shared_ptr.hpp"
#include <cstdio>
#include <cstdarg>

/// \brief strus toplevel namespace
namespace strus
{

/// \class ProcessDebugTrace
/// \brief Error buffer context for one thread
class ProcessDebugTrace
{
public:
	ProcessDebugTrace();
	~ProcessDebugTrace();

	void open( const char* component, const char* type, const std::string& name);
	void close( const char* component);
	void event( const char* component, const char* eventid, const char* format, va_list arg);

	bool enable( const std::string& component);
	void disable( const std::string& component);

	bool isenabled( const char* component) const;

	bool hasError() const;
	std::vector<DebugTraceMessage> fetchMessages();

private:
	bool checkThreadId();

private:
	strus::ThreadId::Type m_tid;
	AtomicFlag m_hasError;
	std::vector<int> m_stk;
	std::vector<DebugTraceMessage> m_msgar;
	enum {SizeOfEnableList = 256};
	char* m_enablelist[ SizeOfEnableList];
	std::size_t m_enablelistsize;
};


class DebugTraceContext
	:public DebugTraceContextInterface
{
public:
	DebugTraceContext( const char* component_, strus::shared_ptr<ProcessDebugTrace>& ref_)
		:m_component(component_),m_ref(ref_){}
	virtual ~DebugTraceContext(){}

	virtual void open( const char* type, const std::string& name);
	virtual void close();
	virtual void event( const char* eventid, const char* format, ...);

private:
	const char* m_component;
	strus::shared_ptr<ProcessDebugTrace> m_ref;
};

/// \class DebugTrace
class DebugTrace
	:public DebugTraceInterface
{
public:
	enum {DefaultMaxNofThreads=32};

	explicit DebugTrace( std::size_t maxNofThreads_);
	virtual ~DebugTrace();

	virtual bool setMaxNofThreads( unsigned int maxNofThreads);

	bool enable( const std::string& component);
	void disable( const std::string& component);

	bool isenabled( const char* component) const;

	virtual DebugTraceContextInterface* createTraceContext( const char* component) const;

	virtual void allocContext();
	virtual void releaseContext();

	virtual std::vector<DebugTraceMessage> fetchMessages() const;
	virtual bool hasError() const;

private:
	int threadidx() const;
	bool initMaxNofThreads( unsigned int maxNofThreads);
	void clearBuffers();

private:
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

	mutable Slot* m_slots;
	mutable strus::shared_ptr<ProcessDebugTrace>* m_ar;
};

}//namespace
#endif

