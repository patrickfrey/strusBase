/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some helpers for command line processing
#ifndef _STRUS_CMDLINEOPT_HPP_INCLUDED
#define _STRUS_CMDLINEOPT_HPP_INCLUDED
#include <string>
#include <utility>
#include <ostream>

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Print a multiline string with indentiation
/// \param[in,out] out print target
/// \param[in] indentsize number of steps to indent
/// \param[in] descr what to print
/// \param[in,out] errorhnd buffer for exception error messages
void printIndentMultilineString( std::ostream& out, std::size_t indentsize, const char* descr, ErrorBufferInterface* errorhnd);

}
#endif


