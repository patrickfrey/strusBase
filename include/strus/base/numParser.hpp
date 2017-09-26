/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some functions to parse a number from a string
#ifndef _STRUS_NUM_PARSER_HPP_INCLUDED
#define _STRUS_NUM_PARSER_HPP_INCLUDED
#include "strus/base/stdint.h"
#include <string>

namespace strus {

enum NumParseError {
	NumParserOk = 0x0,
	NumParserErrNoMem = 0x1,
	NumParserErrConversion = 0x2,
	NumParserErrOutOfRange = 0x3
};

double doubleFromString( const std::string& numstr, NumParseError& err);
double doubleFromString( const char* numstr, std::size_t numsize, NumParseError& err);

int64_t intFromString( const std::string& numstr, uint64_t maxvalue, NumParseError& err);
int64_t intFromString( const char* numstr, std::size_t numsize, uint64_t maxvalue, NumParseError& err);

uint64_t uintFromString( const std::string& numstr, uint64_t maxvalue, NumParseError& err);
uint64_t uintFromString( const char* numstr, std::size_t numsize, uint64_t maxvalue, NumParseError& err);

}//namespace
#endif

