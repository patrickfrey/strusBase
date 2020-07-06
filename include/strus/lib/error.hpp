/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus error library implementing the standard error buffer interface
/// \file error.hpp
#ifndef _STRUS_STORAGE_ERROR_LIB_HPP_INCLUDED
#define _STRUS_STORAGE_ERROR_LIB_HPP_INCLUDED
#include "strus/errorCodes.hpp"
#include <cstdio>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DebugTraceInterface;

/// \brief Create an debug trace interface
/// \param[in] maximum number of threads simultaneously writing to and reading from the error buffer interface
DebugTraceInterface* createDebugTrace_standard( std::size_t maxNofThreads_);

/// \brief Dump the trace content in a convenient form to a file specified
/// \param[in] debugTrace debug trace to dump for the current thread
/// \param[in] filename name of file to dump to, stderr if NULL is specified
bool dumpDebugTrace( DebugTraceInterface* debugTrace, const char* filename);

/// \brief Create an error buffer interface
/// \param[in] logfilehandle where to log error messages
/// \param[in] maximum number of threads simultaneously writing to and reading from the error buffer interface
/// \param[in] debugTrace debug trace interface to use or NULL if undefined, (passed with ownership)
ErrorBufferInterface* createErrorBuffer_standard( FILE* logfilehandle, std::size_t maxNofThreads_, DebugTraceInterface* debugTrace);

/// \brief Declare a pointer to an error buffer interface as singleton
/// \remark The caller of this function has to guarantee that the destructor of the error buffer declared as singleton is called after all objects borrowing it with 'borrowErrorBuffer_singleton()'
void declareErrorBuffer_singleton( ErrorBufferInterface* errbuf);

/// \brief Borrow a reference the error buffer interface declared as singleton
ErrorBufferInterface* borrowErrorBuffer_singleton();

/// \brief Get the next error code in an error message printed by the standard error buffer implementation provided here
/// \param[in,out] msgitr pointer to error message in, pointer to position after the error code extracted
/// \return error code >= 0 or -1 if no error code found
int errorCodeFromMessage( char const*& msgitr);

/// \brief Remove all error codes in an error message printed by the standard error buffer implementation provided here
/// \param[in] msg error message string to process
void removeErrorCodesFromMessage( char* msg);

/// \brief Get the error message as string
/// \param[in] errcode error number (errno or strus::ErrorCode)
/// \return error message string
const char* errorCodeToString( int errcode);

}//namespace
#endif

