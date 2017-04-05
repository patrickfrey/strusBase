/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some functions to parse a number from a string
#include "strus/base/numParser.hpp"
#include "strus/base/stdint.h"
#include "strus/base/dll_tags.hpp"
#include <string>
#include <limits>

using namespace strus;

DLL_PUBLIC double strus::doubleFromString( const char* numstr, std::size_t numsize, NumParseError& err)
{
	err = NumParserOk;
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
				err = NumParserErrConversion;
				return 0.0;
			}
			got_dot = true;
			frac = 0.1;
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
		err = NumParserErrConversion;
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
	err = NumParserOk;
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
			rt = NumParserErrOutOfRange;
			return 0;
		}
	}
	if (ci != ce)
	{
		rt = NumParserErrConversion;
		return 0;
	}
	if (rt > maxvalue)
	{
		rt = NumParserErrOutOfRange;
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
	err = NumParserOk;
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


