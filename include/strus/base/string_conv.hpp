/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some string conversion functions
#ifndef _STRUS_STRING_CONV_HPP_INCLUDED
#define _STRUS_STRING_CONV_HPP_INCLUDED
#include <string>
#include <stdexcept>

namespace strus {

enum StringConvError {
	StringConvOk = 0x0,
	StringConvErrNoMem = 0x1,
	StringConvErrConversion = 0x2
};

/// \brief Convert string conversion error code into an exception
/// \param[in] errcode error code
std::runtime_error stringconv_exception( StringConvError errcode);

/// \brief Convert ASCII letters in string to lowercase
/// \param[in] val input string
/// \param[out] err error code in case of error (not set on success)
/// \return converted string or empty string in case of error
std::string tolower( const char* val, StringConvError& err);

/// \brief Convert ASCII letters in string to lowercase
/// \param[in] val input string
/// \param[in] size input string size in bytes
/// \param[out] err error code in case of error (not set on success)
/// \return converted string or empty string in case of error
std::string tolower( const char* val, std::size_t size, StringConvError& err);

/// \brief Convert ASCII letters in string to lowercase
/// \param[in] val input string
/// \param[out] err error code in case of error (not set on success)
/// \return converted string or empty string in case of error
std::string tolower( const std::string& val, StringConvError& err);

/// \brief Trim trailing and heading whitespace and control characters
/// \param[in] val input string
/// \param[out] err error code in case of error (not set on success)
/// \return converted string or empty string in case of error
std::string trim( const std::string& val, StringConvError& err);

/// \brief Trim trailing and heading whitespace and control characters
/// \param[in] val input string
/// \param[in] size input string size in bytes
/// \param[out] err error code in case of error (not set on success)
/// \return converted string or empty string in case of error
std::string trim( const char* val, std::size_t size, StringConvError& err);

/// \brief Evaluate if a string is empty or contains only space characters
/// \param[in] val pointer to input string
/// \param[in] size size of to input string in bytes
/// \return true if string is empty
bool isEmptyString( const char* val, std::size_t size);

/// \brief Evaluate if a string is empty or contains only space characters
/// \param[in] val input string
/// \return true if string is empty
bool isEmptyString( const std::string& val);

/// \brief Compare on Ascii letter caseinsensitive equality
/// \param[in] val1 input string
/// \param[in] val2 other input string
/// \return true if strings are equal, false else
bool caseInsensitiveEquals( const std::string& val1, const std::string& val2);

/// \brief Compare on Ascii letter caseinsensitive equality
/// \param[in] val1 input string
/// \param[in] val2 other input string
/// \return true if strings are equal, false else
bool caseInsensitiveEquals( const std::string& val1, const char* val2);

/// \brief Compare on Ascii letter caseinsensitive equality
/// \param[in] val1 pointer to input string
/// \param[in] val2 pointer to other input string
/// \return true if strings are equal, false else
bool caseInsensitiveEquals( const char* val1, const char* val2);

/// \brief Test prefix on Ascii letter caseinsensitive equality
/// \param[in] val input string
/// \param[in] prefix prefix to check
/// \return true if string starts with prefix, false else
bool caseInsensitiveStartsWith( const std::string& val, const std::string& prefix);

/// \brief Test prefix on Ascii letter caseinsensitive equality
/// \param[in] val input string
/// \param[in] prefix prefix to check
/// \return true if string starts with prefix, false else
bool caseInsensitiveStartsWith( const std::string& val, const char* prefix);

/// \brief Test prefix on equality
/// \param[in] val input string
/// \param[in] prefix prefix to check
/// \return true if string starts with prefix, false else
bool stringStartsWith( const std::string& val, const std::string& prefix);

/// \brief Convert possibly broken UTF-8 to valid UTF-8
/// \param[in] val input string
/// \param[out] err error code in case of error (not set on success)
/// \return converted string or empty string in case of error
std::string utf8clean( const std::string& name, StringConvError& err);

/// \brief Convert excaped control characters to their unescaped form (e.g. \\n to \n)
/// \param[in] val input string
/// \param[out] err error code in case of error (not set on success)
/// \return converted string or empty string in case of error
std::string unescape( const std::string& val, StringConvError& err);

/// \brief Inlined version of string conversion functions throwing an exception instead of setting an error code on failure
struct string_conv
{
	static std::string tolower( const std::string& str)
	{
		StringConvError errcode = StringConvOk;
		std::string rt = strus::tolower( str, errcode);
		if (errcode != StringConvOk) throw strus::stringconv_exception( errcode);
		return rt;
	}
	static std::string tolower( const char* str, std::size_t strsize)
	{
		StringConvError errcode = StringConvOk;
		std::string rt = strus::tolower( str, strsize, errcode);
		if (errcode != StringConvOk) throw strus::stringconv_exception( errcode);
		return rt;
	}
	static std::string trim( const std::string& str)
	{
		StringConvError errcode = StringConvOk;
		std::string rt = strus::trim( str, errcode);
		if (errcode != StringConvOk) throw strus::stringconv_exception( errcode);
		return rt;
	}
	static std::string trim( const char* str, std::size_t strsize)
	{
		StringConvError errcode = StringConvOk;
		std::string rt = strus::trim( str, strsize, errcode);
		if (errcode != StringConvOk) throw strus::stringconv_exception( errcode);
		return rt;
	}
	static std::string unescape( const std::string& str)
	{
		StringConvError errcode = StringConvOk;
		std::string rt = strus::unescape( str, errcode);
		if (errcode != StringConvOk) throw strus::stringconv_exception( errcode);
		return rt;
	}
};

}//namespace
#endif
