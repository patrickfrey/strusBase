/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for reporting debug trace messages in a uniform way
/// \file debugTraceInterface.hpp
#ifndef _STRUS_DEBUG_TRACE_INTERFACE_HPP_INCLUDED
#define _STRUS_DEBUG_TRACE_INTERFACE_HPP_INCLUDED
#include "strus/errorCodes.hpp"
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus
{

/// \class DebugTraceMessage
/// \brief One item of a debug trace
struct DebugTraceMessage
{
	enum Type {Open, Close, Event};

	DebugTraceMessage( Type type_, const char* component_, const char* id_, const std::string& content_)
		:m_type(type_),m_component(component_),m_id(id_),m_content(content_){}
	DebugTraceMessage( const DebugTraceMessage& o)
		:m_type(o.m_type),m_component(o.m_component),m_id(o.m_id),m_content(o.m_content){}

	static const char* typeName( Type tp)
	{
		static const char* ar[] = {"open","close","event"};
		return ar[tp];
	}

	Type type() const			{return m_type;}
	const char* component() const		{return m_component;}
	const char* id() const			{return m_id;}
	const std::string& content() const	{return m_content;}

	const char* typeName() const		{return typeName( m_type);}

private:
	Type m_type;
	const char* m_component;
	const char* m_id;
	std::string m_content;
};

/// \class DebugTraceContextInterface
/// \brief Interface for reporting debug trace messages in a uniform way
class DebugTraceContextInterface
{
public:
	/// \brief Destructor
	virtual ~DebugTraceContextInterface(){}

	/// \brief Hierarchical open of a scope of events
	/// \param[in] type type of the structure
	/// \param[in] name name of the structure
	/// \remark must not throw
	virtual void open( const char* type, const std::string& name=std::string())=0;

	/// \brief Close of the current scope
	virtual void close()=0;

	/// \brief Report an event
	/// \param[in] eventid event id static const string describing the type of an event
	/// \param[in] format error message format string
	/// \remark must not throw
	virtual void event( const char* eventid, const char* format, ...)
#ifdef __GNUC__
	__attribute__ ((format (printf, 3, 4)))
#endif
	=0;
};

/// \class DebugTraceInterface
/// \brief Managing interface for reporting debug trace messages in a uniform way
class DebugTraceInterface
{
public:
	/// \brief Destructor
	virtual ~DebugTraceInterface(){}

	/// \brief Redefine the maximum number of threads using the debug trace
	/// \param[in] maxNofThreads number of threads
	/// \remark not thread safe, may invalidate all objects dependent on this, assumed not to be called outside the main process initialization phase
	virtual bool setMaxNofThreads( unsigned int maxNofThreads)=0;

	/// \brief Enable debugging for a component adressed by name
	/// \param[in] component name of component to enable debugging for
	/// \return true on success, false if an occurred (memory or resource allocation error)
	/// \remark must not throw
	virtual bool enable( const std::string& component)=0;

	/// \brief Disable debugging for a component adressed by name if enabled before
	/// \param[in] component name of component to disable debugging for
	virtual void disable( const std::string& component)=0;

	/// \brief Get the context for a component addressed by name if enabled for the current thread 
	/// \param[in] component name of component queried
	/// \return the debug trace context for a component or NULL if not enabled
	/// \remark must not throw
	virtual DebugTraceContextInterface* createTraceContext( const char* component) const=0;

	/// \brief Allocate context for current thread
	/// \remark allocContext & releaseContext have only to be called if assignement of thread ids is not fix
	virtual void allocContext()=0;

	/// \brief Dellocate context for current thread
	/// \remark allocContext & releaseContext have only to be called if assignement of thread ids is not fix
	virtual void releaseContext()=0;

	/// \brief Test if an error occurred in the trace
	virtual bool hasError() const=0;

	/// \brief Get and clear all messages of the current thread an clears the error flag
	virtual std::vector<DebugTraceMessage> fetchMessages() const=0;
};

}//namespace
#endif

