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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <stdlib.h>

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

DLL_PUBLIC std::string strus::tolower( const char* val, StringConvError& err)
{
	try
	{
		std::size_t len = std::strlen(val)+1;
		std::string rt;
		rt.reserve( len);
		char const* vv = val;
		while (*vv) rt.push_back( std::tolower(*vv++));
		return rt;
	}
	catch (const std::bad_alloc&)
	{
		err = StringConvErrNoMem;
		return std::string();
	}
}

DLL_PUBLIC std::string strus::tolower( const std::string& val, StringConvError& err)
{
	try
	{
		return boost::algorithm::to_lower_copy( val);
	}
	catch (const std::bad_alloc&)
	{
		err = StringConvErrNoMem;
		return std::string();
	}
}

DLL_PUBLIC std::string strus::trim( const std::string& val, StringConvError& err)
{
	try
	{
		return boost::algorithm::trim_copy( val);
	}
	catch (const std::bad_alloc&)
	{
		err = StringConvErrNoMem;
		return std::string();
	}
}

DLL_PUBLIC bool strus::caseInsensitiveEquals( const std::string& val1, const std::string& val2)
{
	try
	{
		return boost::algorithm::iequals( val1, val2);
	}
	catch (...)
	{
		return false;
	}
}

DLL_PUBLIC bool strus::caseInsensitiveStartsWith( const std::string& val, const std::string& prefix)
{
	try
	{
		return boost::algorithm::istarts_with( val, prefix);
	}
	catch (...)
	{
		return false;
	}
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

