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

/// \brief Create an error buffer interface
/// \param[in] logfilehandle where to log error messages
/// \param[in] maximum number of threads simultaneously writing to and reading from the error buffer interface
ErrorBufferInterface* createErrorBuffer_standard( FILE* logfilehandle, std::size_t maxNofThreads_);

/// \brief Get the next error code in an error message printed by the standard error buffer implementation provided here
/// \param[in,out] msgitr pointer to error message in, pointer to position after the error code extracted
/// \return error code >= 0 or -1 if no error code found
int errorCodeFromMessage( char const*& msgitr);

/// \brief Remove all error codes in an error message printed by the standard error buffer implementation provided here
/// \param[in] msg error message string to process
void removeErrorCodesFromMessage( char* msg);

/// \brief Get the name of an error component id as string
/// \param[in] component component id
/// \return component name as string ("other" if unknown)
const char* errorComponentName( ErrorComponent component);

}//namespace
#endif

