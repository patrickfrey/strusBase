/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for reporting and catching errors
/// \file errorBufferInterface.hpp
#ifndef _STRUS_BASE_ERROR_BUFFER_INTERFACE_HPP_INCLUDED
#define _STRUS_BASE_ERROR_BUFFER_INTERFACE_HPP_INCLUDED
#include "strus/errorCodes.hpp"
#include "strus/debugTraceInterface.hpp"
#include <cstdio>

/// \brief strus toplevel namespace
namespace strus
{

/// \class ErrorBufferInterface
/// \brief Interface for reporting and catching errors in modules
/// \note C++ Coding Standards: 101 Rules, Guidelines, and Best Practices by Andrei Alexandrescu, Herb Sutter:
///	62. Don’t allow exceptions to propagate across module boundaries
class ErrorBufferInterface
{
public:
	enum ErrorClass
	{
		None,		///< no error
		RuntimeError,	///< runtime error
		BadAlloc	///< memory allocation error
	};

	/// \brief Destructor
	virtual ~ErrorBufferInterface(){}

	/// \brief Define error log file
	/// \param[in] hnd file handle to set as log file
	/// \remark not thread safe, assumed not to be called outside the main process initialization phase
	virtual void setLogFile( FILE* hnd)=0;

	/// \brief Redefine the maximum number of threads using the error buffer
	/// \param[in] maxNofThreads number of threads
	/// \remark not thread safe, may invalidate all objects dependent on this, assumed not to be called outside the main process initialization phase
	virtual bool setMaxNofThreads( unsigned int maxNofThreads)=0;

	/// \brief Report an error
	/// \param[in] errorcode error code of the error (errno and beyond)
	/// \note See include/strus/errorCodes.hpp
	/// \param[in] format error message format string
	/// \remark must not throw
	virtual void report( int errorcode, const char* format, ...)
#ifdef __GNUC__
	__attribute__ ((format (printf, 3, 4)))
#endif
	=0;
	/// \brief Report an error, overwriting the previous error
	/// \param[in] format error message format string
	/// \remark must not throw
	virtual void explain( const char* format)=0;

	/// \brief Check, if an error has occurred and return it
	/// \return an error string, if defined, NULL else
	/// \remark resets the error
	virtual const char* fetchError()=0;

	/// \brief Check, if an error has occurred
	/// \return an error string, if defined, NULL else
	virtual bool hasError() const=0;

	/// \brief Allocate context for current thread
	/// \remark allocContext & releaseContext have only to be called if assignement of thread ids is not fix
	virtual void allocContext()=0;

	/// \brief Dellocate context for current thread
	/// \remark allocContext & releaseContext have only to be called if assignement of thread ids is not fix
	virtual void releaseContext()=0;

	/// \brief Fetches the debug trace interface if defined
	/// \return the debug trace interface if defined, NULL else
	virtual DebugTraceInterface* debugTrace() const=0;
};

}//namespace
#endif

