/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some string conversion functions
#include "strus/base/string_conv.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/utf8.hpp"
#include "private/internationalization.hpp"
#include <string>
#include <stdexcept>
#include <stdlib.h>
#include <cctype>

using namespace strus;

DLL_PUBLIC std::runtime_error strus::stringconv_exception( StringConvError errcode)
{
	switch (errcode)
	{
		case StringConvOk: return std::runtime_error( _TXT("ok"));
		case StringConvErrNoMem: return std::runtime_error( _TXT("out of memory in string conversion"));
		case StringConvErrConversion: return std::runtime_error( _TXT("string conversion error"));
	}
	return std::runtime_error( _TXT("uncaught string conversion error"));
}

DLL_PUBLIC std::string strus::tolower( const char* val, std::size_t size, StringConvError& err)
{
	try
	{
		std::string rt;
		rt.reserve( size+1);
		char const* vi = val;
		const char* ve = vi  + size;
		for (; vi != ve; ++vi)
		{
			rt.push_back( std::tolower( *vi));
		}
		return rt;
	}
	catch (const std::bad_alloc&)
	{
		err = StringConvErrNoMem;
		return std::string();
	}
}

DLL_PUBLIC std::string strus::tolower( const char* val, StringConvError& err)
{
	return strus::tolower( val, std::strlen(val), err);
}

DLL_PUBLIC std::string strus::tolower( const std::string& val, StringConvError& err)
{
	return strus::tolower( val.c_str(), val.size(), err);
}

DLL_PUBLIC std::string strus::toupper( const char* val, std::size_t size, StringConvError& err)
{
	try
	{
		std::string rt;
		rt.reserve( size+1);
		char const* vi = val;
		const char* ve = vi  + size;
		for (; vi != ve; ++vi)
		{
			rt.push_back( std::toupper( *vi));
		}
		return rt;
	}
	catch (const std::bad_alloc&)
	{
		err = StringConvErrNoMem;
		return std::string();
	}
}

DLL_PUBLIC std::string strus::toupper( const char* val, StringConvError& err)
{
	return strus::toupper( val, std::strlen(val), err);
}

DLL_PUBLIC std::string strus::toupper( const std::string& val, StringConvError& err)
{
	return strus::toupper( val.c_str(), val.size(), err);
}

DLL_PUBLIC std::string strus::trim( const char* val, std::size_t size, StringConvError& err)
{
	try
	{
		std::string rt;
		char const* vi = val;
		const char* ve = vi + size;
		for (; vi != ve; ++vi)
		{
			if ((unsigned char)*vi > 32) break;
		}
		const char* start = vi;
		char const* last = ve;
		while (last != start)
		{
			--last;
			if ((unsigned char)*last > 32)
			{
				++last;
				break;
			}
		}
		return std::string( start, last-start);
	}
	catch (const std::bad_alloc&)
	{
		err = StringConvErrNoMem;
		return std::string();
	}
}

DLL_PUBLIC std::string strus::trim( const std::string& val, StringConvError& err)
{
	return strus::trim( val.c_str(), val.size(), err);
}

DLL_PUBLIC bool strus::isEmptyString( const char* val, std::size_t size)
{
	char const* vi = val;
	const char* ve = vi + size;
	for (; vi != ve && (unsigned char)*vi <= 32; ++vi){}
	return vi == ve;
}

DLL_PUBLIC bool strus::isEmptyString( const std::string& val)
{
	return isEmptyString( val.c_str(), val.size());
}

static bool caseInsensitiveEquals_( char const* vi, char const* oi)
{
	for (; *vi && *oi && std::tolower( *vi) == std::tolower( *oi); ++vi,++oi){}
	return 0==(*vi|*oi);
}

DLL_PUBLIC bool strus::caseInsensitiveEquals( const std::string& val1, const std::string& val2)
{
	if (val1.size() != val2.size()) return false;
	return caseInsensitiveEquals_( val1.c_str(), val2.c_str());
}

DLL_PUBLIC bool strus::caseInsensitiveEquals( const std::string& val1, const char* val2)
{
	return caseInsensitiveEquals_( val1.c_str(), val2);
}

DLL_PUBLIC bool strus::caseInsensitiveEquals( const char* val1, const char* val2)
{
	return caseInsensitiveEquals_( val1, val2);
}

static bool caseInsensitiveStartsWith_( char const* vi, char const* pi)
{
	for (; *vi && std::tolower( *vi) == std::tolower( *pi); ++vi,++pi){}
	return 0==*pi;
}

DLL_PUBLIC bool strus::caseInsensitiveStartsWith( const std::string& val, const std::string& prefix)
{
	if (val.size() < prefix.size()) return false;
	return caseInsensitiveStartsWith_( val.c_str(), prefix.c_str());
}

DLL_PUBLIC bool strus::caseInsensitiveStartsWith( const std::string& val, const char* prefix)
{
	return caseInsensitiveStartsWith_( val.c_str(), prefix);
}

DLL_PUBLIC bool strus::stringStartsWith( const std::string& val, const std::string& prefix)
{
	std::string rt;
	if (val.size() < prefix.size()) return false;
	std::string::const_iterator vi = val.begin();
	std::string::const_iterator pi = prefix.begin(), pe = prefix.end();
	for (; pi != pe; ++pi,++vi)
	{
		if (*vi != *pi) return false;
	}
	return pi == pe;
}

DLL_PUBLIC std::string strus::utf8clean( const std::string& name, StringConvError& err)
{
	try
	{
		std::string rt;
		std::size_t si = 0, se = name.size();
		while (si < se)
		{
			unsigned char asciichr = name[si];
			if (asciichr && asciichr < 128)
			{
				rt.push_back( asciichr);
				++si;
			}
			else
			{
				std::size_t chrlen = strus::utf8charlen( asciichr);
				uint32_t chr = strus::utf8decode( name.c_str() + si, chrlen);
				if (chr)
				{
					char buf[ 16];
					chrlen = strus::utf8encode( buf, chr);
					rt.append( buf, chrlen);
				}
				si += chrlen;
			}
		}
		return rt;
	}
	catch (const std::bad_alloc&)
	{
		err = StringConvErrNoMem;
		return std::string();
	}
}

DLL_PUBLIC std::string strus::unescape( const std::string& val, StringConvError& err)
{
	try
	{
		std::string rt;
		std::string::const_iterator vi = val.begin(), ve = val.end();
		for (; vi != ve; ++vi)
		{
			if (*vi == '\\')
			{
				++vi;
				if (*vi == 'n') rt.push_back('\n');
				else if (*vi == 'a') rt.push_back('\a');
				else if (*vi == 'b') rt.push_back('\b');
				else if (*vi == 't') rt.push_back('\t');
				else if (*vi == 'r') rt.push_back('\r');
				else if (*vi == 'f') rt.push_back('\f');
				else if (*vi == 'v') rt.push_back('\v');
				else if (*vi == '\\') rt.push_back('\\');
				else if (*vi == '0') rt.push_back('\0');
				else throw strus::runtime_error(_TXT("unknown escape character \\%c"), *vi);
			}
			else
			{
				rt.push_back( *vi);
			}
		}
		return rt;
	}
	catch (const std::bad_alloc&)
	{
		err = StringConvErrNoMem;
		return std::string();
	}
}

