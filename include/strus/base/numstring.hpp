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
	NumParseErrOutOfRange = 0x3,
	NumParseErrInvalidArg = 0x4
};

/// \brief Convert string conversion error code into an error message string
/// \param[in] errcode error code
/// \return error message string
const char* numstring_error( NumParseError errcode);

/// \brief Convert string conversion error code into an exception
/// \param[in] errcode error code
/// \return runtime error exception
std::runtime_error numstring_exception( NumParseError errcode);

/// \brief Parsing a double precision floating point number from an Ascii string
/// \param[in] numstr the number to parse as Ascii string
/// \param[out] err error code in case of an error, unchanged in case of success
/// \return the number parsed as a double precision floating point number or 0 in case of an error
double doubleFromString( const std::string& numstr, NumParseError& err);

/// \brief Parsing a double precision floating point number from an Ascii string
/// \param[in] numstr the pointer to the number to parse as Ascii string
/// \param[in] numsize the size of numstr in bytes
/// \param[out] err error code in case of an error, unchanged in case of success
/// \return the number parsed as a double precision floating point number or 0 in case of an error
double doubleFromString( const char* numstr, std::size_t numsize, NumParseError& err);

/// \brief Parsing an integer number from an Ascii string
/// \param[in] numstr the number to parse as Ascii string
/// \param[in] maxvalue the maximum value accepted for the result
/// \param[out] err error code in case of an error, unchanged in case of success
/// \return the number parsed as an integer or 0 in case of an error
int64_t intFromString( const std::string& numstr, int64_t maxvalue, NumParseError& err);

/// \brief Parsing an integer number from an Ascii string
/// \param[in] numstr the pointer to the number to parse as Ascii string
/// \param[in] numsize the size of numstr in bytes
/// \param[in] maxvalue the maximum value accepted for the result
/// \param[out] err error code in case of an error, unchanged in case of success
/// \return the number parsed as an integer or 0 in case of an error
int64_t intFromString( const char* numstr, std::size_t numsize, int64_t maxvalue, NumParseError& err);

/// \brief Parsing an unsigned integer number from an Ascii string
/// \param[in] numstr the number to parse as Ascii string
/// \param[in] maxvalue the maximum value accepted for the result
/// \param[out] err error code in case of an error, unchanged in case of success
/// \return the number parsed as an unsigned integer or 0 in case of an error
uint64_t uintFromString( const std::string& numstr, uint64_t maxvalue, NumParseError& err);

/// \brief Parsing an unsigned integer number from an Ascii string
/// \param[in] numstr the pointer to the number to parse as Ascii string
/// \param[in] numsize the size of numstr in bytes
/// \param[in] maxvalue the maximum value accepted for the result
/// \param[out] err error code in case of an error, unchanged in case of success
/// \return the number parsed as an unsigned integer or 0 in case of an error
uint64_t uintFromString( const char* numstr, std::size_t numsize, uint64_t maxvalue, NumParseError& err);

/// \brief Inlined version of string to number conversion functions throwing an exception instead of setting an error code on failure
struct numstring_conv
{
	/// \brief Parsing an integer number from an Ascii string, throwing in case of error
	/// \param[in] str the number to parse as Ascii string
	/// \param[in] maxval the maximum value accepted for the result
	/// \return the number parsed as an integer
	static int64_t toint( const std::string& str, int64_t maxval)
	{
		NumParseError errcode = NumParseOk;
		int64_t rt = strus::intFromString( str, maxval, errcode);
		if (errcode != NumParseOk) throw strus::numstring_exception( errcode);
		return rt;
	}
	/// \brief Parsing an integer number from an Ascii string, throwing in case of error
	/// \param[in] str the number to parse as Ascii string
	/// \param[in] len length of str in bytes
	/// \param[in] maxval the maximum value accepted for the result
	/// \return the number parsed as an integer
	static int64_t toint( const char* str, std::size_t len, int64_t maxval)
	{
		NumParseError errcode = NumParseOk;
		int64_t rt = strus::intFromString( str, len, maxval, errcode);
		if (errcode != NumParseOk) throw strus::numstring_exception( errcode);
		return rt;
	}
	/// \brief Parsing an unsigned integer number from an Ascii string, throwing in case of error
	/// \param[in] str the number to parse as Ascii string
	/// \param[in] maxval the maximum value accepted for the result
	/// \return the number parsed as an unsigned integer
	static uint64_t touint( const std::string& str, uint64_t maxval)
	{
		NumParseError errcode = NumParseOk;
		uint64_t rt = strus::uintFromString( str, maxval, errcode);
		if (errcode != NumParseOk) throw strus::numstring_exception( errcode);
		return rt;
	}
	/// \brief Parsing an unsigned integer number from an Ascii string, throwing in case of error
	/// \param[in] str the number to parse as Ascii string
	/// \param[in] len length of str in bytes
	/// \param[in] maxval the maximum value accepted for the result
	/// \return the number parsed as an unsigned integer
	static uint64_t touint( const char* str, std::size_t len, uint64_t maxval)
	{
		NumParseError errcode = NumParseOk;
		uint64_t rt = strus::uintFromString( str, len, maxval, errcode);
		if (errcode != NumParseOk) throw strus::numstring_exception( errcode);
		return rt;
	}
	/// \brief Parsing a double precision floating point number from an Ascii string, throwing in case of error
	/// \param[in] str the number to parse as Ascii string
	/// \return the number parsed as an unsigned integer
	static double todouble( const std::string& str)
	{
		NumParseError errcode = NumParseOk;
		double rt = strus::doubleFromString( str, errcode);
		if (errcode != NumParseOk) throw strus::numstring_exception( errcode);
		return rt;
	}
	/// \brief Parsing a double precision floating point number from an Ascii string, throwing in case of error
	/// \param[in] str the number to parse as Ascii string
	/// \param[in] len length of str in bytes
	/// \return the number parsed as an unsigned integer
	static double todouble( const char* str, std::size_t len)
	{
		NumParseError errcode = NumParseOk;
		double rt = strus::doubleFromString( str, len, errcode);
		if (errcode != NumParseOk) throw strus::numstring_exception( errcode);
		return rt;
	}
};

}//namespace
#endif

