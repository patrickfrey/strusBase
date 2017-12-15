/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Variant type for fixed size numeric types (integers, floating point numbers)
/// \file numericVariant.cpp
#include "strus/numericVariant.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/numstring.hpp"

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <limits>

using namespace strus;

static bool unsignedIntFromString_( uint64_t& res, const char* numstr, std::size_t numsize)
{
	res = 0;
	uint64_t res_prev = 0;
	char const* ci = numstr;
	const char* ce = numstr + numsize;
	for (;ci != ce; ++ci)
	{
		res_prev = res;
		if (*ci >= '0' && *ci <= '9')
		{
			res = (res * 10) + (*ci - '0');
		}
		if (res_prev > res)
		{
			return false;
		}		
	}
	return true;
}

static bool signedIntFromString_( int64_t& res, const char* numstr, std::size_t numsize)
{
	uint64_t ures = 0;
	if (numsize > 0 && numstr[0] == '-')
	{
		if (!unsignedIntFromString_( ures, numstr+1, numsize-1) || ures > (uint64_t)((uint64_t)std::numeric_limits<int64_t>::max()+1)) return false;
		if (ures == 0)
		{
			res = 0;
		}
		else
		{
			res = -(int64_t)(ures-1)-1;
		}
	}
	else
	{
		if (!unsignedIntFromString_( ures, numstr, numsize) || ures > std::numeric_limits<int64_t>::max()) return false;
		res = ures;
	}
	return true;
}

static bool doubleFloatFromString_( double& res, const char* numstr, std::size_t numsize)
{
	NumParseError err = NumParseOk;
	res = doubleFromString( numstr, numsize, err);
	return (err == NumParseOk);
}

DLL_PUBLIC NumericVariant::String::String( const NumericVariant& val, int precision)
{
	switch (val.type)
	{
		case Null: break;
		case Int: snprintf( m_buf, sizeof(m_buf), "%" PRId64, val.variant.Int); return;
		case UInt: snprintf( m_buf, sizeof(m_buf), "%" PRIu64, val.variant.UInt); return;
		case Float:
			if (precision > 0)
			{
				snprintf( m_buf, sizeof(m_buf), "%.*g", precision, val.variant.Float);
				return;
			}
			else
			{
				snprintf( m_buf, sizeof(m_buf), "%.16g", val.variant.Float);
				return;
			}
	}
	m_buf[0] = '\0';
}

DLL_PUBLIC bool NumericVariant::initFromString( const char* src)
{
	char const* si = src;
	bool sign_ = false;
	if (!*si)
	{
		init();
		return true;
	}
	if (*si == '-')
	{
		sign_ = true;
		++si;
	}
	if (*si < '0' || *si > '9') return false;
	for (; *si >= '0' && *si <= '9'; ++si){}
	if (*si == '.')
	{
		double val = 0;
		if (doubleFloatFromString_( val, src, std::strlen(src)))
		{
			type = Float;
			variant.Float = val;
			return true;
		}
	}
	else if (sign_)
	{
		int64_t val = 0;
		if (signedIntFromString_( val, src, std::strlen(src)))
		{
			type = Int;
			variant.Int = val;
			return true;
		}
	}
	else
	{
		uint64_t val = 0;
		if (unsignedIntFromString_( val, src, std::strlen(src)))
		{
			type = UInt;
			variant.UInt = val;
			return true;
		}
	}
	return false;
}

DLL_PUBLIC bool NumericVariant::isequal( const NumericVariant& o) const
{
	if (type == o.type)
	{
		switch (type)
		{
			case Null: return true;
			case Int: return variant.Int == o.variant.Int;
			case UInt: return variant.UInt == o.variant.UInt;
			case Float:
			{
				double diff;
				double a = variant.Float;
				double b = o.variant.Float;
				if (a > 1E3 || a < -1E3)
				{
					diff = 1.0 - a/b;
					if (diff < 0.0) diff = -diff;
				}
				else
				{
					diff = a > b ? (a - b) : (b - a);
				}
				return (diff < std::numeric_limits<double>::epsilon()*100);
			}
		}
	}
	return false;
}

DLL_PUBLIC int NumericVariant::compare( const NumericVariant& o) const
{
	switch (type)
	{
		case Null: return o.defined() ? -1:0;
		case Int: 
		{
			int64_t vv = o.toint();
			if (variant.Int < vv)
			{
				return -1;
			}
			else if (variant.Int > vv)
			{
				return +1;
			}
			else
			{
				return 0;
			}
		}
		case UInt:
		{
			uint64_t vv = o.touint();
			if (variant.UInt < vv)
			{
				return -1;
			}
			else if (variant.UInt > vv)
			{
				return +1;
			}
			else
			{
				return 0;
			}
		}
		case Float:
		{
			double vv = o.tofloat();
			double xx = variant.Float - vv;
			double xxabs = (xx > 0.0) ? xx:-xx;
			if (xxabs <= 100*std::numeric_limits<double>::epsilon()) return 0;
			return xx < vv ? -1:+1;
		}
	}
	return -1;
}

DLL_PUBLIC const char* NumericVariant::typeName( Type type)
{
	switch (type)
	{
		case Null: return "NULL";
		case Int: return "INT";
		case UInt: return "UINT";
		case Float: return "FLOAT";
	}
	return 0;
}


