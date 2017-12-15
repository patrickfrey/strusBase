/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some functions to parse a number from a string and to print a number to a string
#ifndef _STRUS_NUM_STRING_HPP_INCLUDED
#define _STRUS_NUM_STRING_HPP_INCLUDED
#include "strus/base/stdint.h"
#include <string>
#include <stdexcept>

namespace strus {

enum NumParseError {
	NumParseOk = 0x0,
	NumParseErrNoMem = 0x1,
	NumParseErrConversion = 0x2,
	NumParseErrOutOfRange = 0x3
};

/// \brief Convert string conversion error code into an exception
/// \param[in] errcode error code
std::runtime_error numstring_exception( NumParseError errcode);

double doubleFromString( const std::string& numstr, NumParseError& err);
double doubleFromString( const char* numstr, std::size_t numsize, NumParseError& err);

int64_t intFromString( const std::string& numstr, uint64_t maxvalue, NumParseError& err);
int64_t intFromString( const char* numstr, std::size_t numsize, uint64_t maxvalue, NumParseError& err);

uint64_t uintFromString( const std::string& numstr, uint64_t maxvalue, NumParseError& err);
uint64_t uintFromString( const char* numstr, std::size_t numsize, uint64_t maxvalue, NumParseError& err);

/// \brief Inlined version of string to number conversion functions throwing an exception instead of setting an error code on failure
struct numstring_conv
{
	static int64_t toint( const std::string& str, int64_t maxval)
	{
		NumParseError errcode = NumParseOk;
		int64_t rt = strus::intFromString( str, maxval, errcode);
		if (errcode != NumParseOk) throw strus::numstring_exception( errcode);
		return rt;
	}
	static uint64_t touint( const std::string& str, uint64_t maxval)
	{
		NumParseError errcode = NumParseOk;
		uint64_t rt = strus::uintFromString( str, maxval, errcode);
		if (errcode != NumParseOk) throw strus::numstring_exception( errcode);
		return rt;
	}
	static double todouble( const std::string& str)
	{
		NumParseError errcode = NumParseOk;
		double rt = strus::doubleFromString( str, errcode);
		if (errcode != NumParseOk) throw strus::numstring_exception( errcode);
		return rt;
	}
};

}//namespace
#endif

