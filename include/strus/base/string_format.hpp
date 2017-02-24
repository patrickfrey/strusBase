/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
///\brief Get a string built by a format string
#ifndef _STRUS_BASE_STRING_FORMAT_HPP_INCLUDED
#define _STRUS_BASE_STRING_FORMAT_HPP_INCLUDED
#include <string>
#include <cstdlib>
#include <stdarg.h>

namespace strus {

/// \brief Get a string built from a format string and arguments
/// \param[in] fmt C printf format string
/// \param[in] ... argument list
/// \return the string built or an empty string in case of a memory allocation error
#ifdef __GNUC__
std::string string_format( const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
#else
std::string string_format( const char* fmt, ...);
#endif

} //namespace
#endif
