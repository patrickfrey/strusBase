/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some functions to parse a number from a string
#include "strus/base/numstring.hpp"
#include "strus/base/stdint.h"
#include "strus/base/dll_tags.hpp"
#include "private/internationalization.hpp"
#include <string>
#include <limits>
#include <cmath>

using namespace strus;

DLL_PUBLIC std::runtime_error strus::numstring_exception( NumParseError errcode)
{
	switch (errcode)
	{
		case NumParseOk: return std::runtime_error( _TXT("ok"));
		case NumParseErrNoMem: return std::runtime_error( _TXT("out of memory in string to number conversion"));
		case NumParseErrConversion: return std::runtime_error( _TXT("string to number conversion error"));
		case NumParseErrOutOfRange: return std::runtime_error( _TXT("parsed number out of range"));
	}
	return std::runtime_error( _TXT("uncaught string to number conversion error"));
}

DLL_PUBLIC double strus::doubleFromString( const char* numstr, std::size_t numsize, NumParseError& err)
{
	err = NumParseOk;
	double rt = 0.0;
	double frac = 0.0;
	bool got_dot = false;
	bool sign = false;
	char const* ci = numstr;
	const char* ce = numstr + numsize;
	if (ci < ce)
	{
		if (*ci == '-')
		{
			sign = true;
			++ci;
		}
		else if (*ci == '+')
		{
			++ci;
		}
	}
	for (;ci != ce; ++ci)
	{
		if (*ci >= '0' && *ci <= '9')
		{
			if (got_dot)
			{
				rt += (double)(unsigned int)(*ci - '0') * frac;
				frac /= 10;
			}
			else
			{
				rt = (rt * 10) + (*ci - '0');
			}
		}
		else if (*ci == '.')
		{
			if (got_dot)
			{
				err = NumParseErrConversion;
				return 0.0;
			}
			got_dot = true;
			frac = 0.1;
		}
		else if (*ci == 'E' || *ci == 'e')
		{
			++ci;
			bool eesign = false;
			if (ci != ce)
			{
				if (*ci == '-')
				{
					eesign = true;
					++ci;
				}
				else if (*ci == '+')
				{
					++ci;
				}
				else
				{
					break;
				}
			}
			else
			{
				--ci;
				break;
			}
			if (ci == ce || *ci < '0' || *ci > '9') break;
			int ee = 0;
			for (; ci != ce && *ci >= '0' && *ci <= '9' && ee < 1000; ++ci)
			{
				ee *= 10;
				ee += *ci - '0';
			}
			if (ee) rt *= std::pow( 10, eesign?-ee:ee);
			break;
		}
		else if (*ci == '%')
		{
			rt /= 100;
			++ci;
			break;
		}
		else
		{
			break;
		}
	}
	if (ci != ce)
	{
		err = NumParseErrConversion;
		return 0.0;
	}
	if (sign) rt = -rt;
	return rt;
}

DLL_PUBLIC double strus::doubleFromString( const std::string& numstr, NumParseError& err)
{
	return strus::doubleFromString( numstr.c_str(), numstr.size(), err);
}

static uint64_t unsignedFromString_( const char* numstr, std::size_t numsize, uint64_t maxvalue, NumParseError& err)
{
	err = NumParseOk;
	uint64_t rt = 0;
	uint64_t rt_prev = 0;
	char const* ci = numstr;
	const char* ce = numstr + numsize;
	for (;ci != ce; ++ci)
	{
		rt_prev = rt;
		if (*ci >= '0' && *ci <= '9')
		{
			rt = (rt * 10) + (*ci - '0');
		}
		else if (*ci == 'K' || *ci == 'k')
		{
			rt = rt * 1024;
			++ci;
			break;
		}
		else if (*ci == 'M' || *ci == 'm')
		{
			rt = rt * 1024 * 1024;
			++ci;
			break;
		}
		else if (*ci == 'G' || *ci == 'g')
		{
			rt = rt * 1024 * 1024 * 1024;
			++ci;
			break;
		}
		else
		{
			break;
		}
		if (rt_prev > rt)
		{
			rt = NumParseErrOutOfRange;
			return 0;
		}
	}
	if (ci != ce)
	{
		rt = NumParseErrConversion;
		return 0;
	}
	if (rt > maxvalue)
	{
		rt = NumParseErrOutOfRange;
		return 0;
	}
	return rt;
}


DLL_PUBLIC uint64_t strus::uintFromString( const char* numstr, std::size_t numsize, uint64_t maxvalue, NumParseError& err)
{
	return unsignedFromString_( numstr, numsize, maxvalue, err);
}

DLL_PUBLIC uint64_t strus::uintFromString( const std::string& numstr, uint64_t maxvalue, NumParseError& err)
{
	return strus::uintFromString( numstr.c_str(), numstr.size(), maxvalue, err);
}

DLL_PUBLIC int64_t strus::intFromString( const char* numstr, std::size_t numsize, uint64_t maxvalue, NumParseError& err)
{
	err = NumParseOk;
	bool sign = false;
	std::size_t ii = 0;
	if (ii < numsize)
	{
		if (numstr[ii] == '-')
		{
			sign = true;
			++ii;
		}
		else if (numstr[ii] == '+')
		{
			++ii;
		}
	}
	uint64_t val = unsignedFromString_( numstr+ii, numsize-ii, maxvalue, err);
	return sign?-(int64_t)val:(int64_t)val;
}

DLL_PUBLIC int64_t strus::intFromString( const std::string& numstr, uint64_t maxvalue, NumParseError& err)
{
	return strus::intFromString( numstr.c_str(), numstr.size(), maxvalue, err);
}

