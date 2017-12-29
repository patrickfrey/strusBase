/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/numericVariant.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include <limits>

using namespace strus;

#define STRUS_LOWLEVEL_DEBUG

static void convString( int64_t& output, const char* valuestr)
{
	NumericVariant value( valuestr);
	if (value.type == NumericVariant::UInt) value.type = NumericVariant::Int;
	if (value.type != NumericVariant::Int) throw std::runtime_error( "failed to convert string to int");
	output = value.variant.Int;
}
static void convString( uint64_t& output, const char* valuestr)
{
	NumericVariant value( valuestr);
	if (value.type != NumericVariant::UInt) throw std::runtime_error( "failed to convert string to int");
	output = value.variant.UInt;
}
static void convString( double& output, const char* valuestr)
{
	NumericVariant value( valuestr);
	if (value.type != NumericVariant::Float) throw std::runtime_error( "failed to convert string to double");
	output = value.variant.Float;
}
template <typename TYPE>
bool compareValue( const TYPE& a, const TYPE& b)
{
	return a == b;
}
template <>
bool compareValue<double>( const double& a, const double& b)
{
	double diff;
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

template <typename INPUT>
bool testToString( int idx, const INPUT& input)
{
	bool rt = false;
	NumericVariant value( input);
	NumericVariant::String str( value);
	std::cerr << "[" << idx << "] convert " << NumericVariant::typeName( value.type) << " '" << input << "' to string '" << str.c_str() << "'";
	INPUT res;
	convString( res, str.c_str());
	if (compareValue( res, input))
	{
		std::cerr << " OK" << std::endl;
		rt = true;
	}
	else
	{
		std::cerr << " DIFF" << std::endl;
		rt = false;
	}
	return rt;
}

int main( int argc, const char* argv[])
{
	if (argc >= 2 && (std::strcmp( argv[1], "-h") == 0 || std::strcmp( argv[1], "--help") == 0))
	{
		std::cerr << "testNumericVariant" << std::endl;
		return 0;
	}
	try
	{
		int testidx = 0;
		int errcnt = 0;
		double PI = std::atan(1)*4; 
		errcnt += (int)!testToString<int64_t>( ++testidx, 0);
		errcnt += (int)!testToString<int64_t>( ++testidx, 1);
		errcnt += (int)!testToString<int64_t>( ++testidx, 1U);
		errcnt += (int)!testToString<int64_t>( ++testidx, (uint64_t)1 << 16);
		errcnt += (int)!testToString<int64_t>( ++testidx, +12212);
		errcnt += (int)!testToString<int64_t>( ++testidx, -31312);
		errcnt += (int)!testToString<uint64_t>( ++testidx, +31312);
		errcnt += (int)!testToString<int64_t>( ++testidx, (uint64_t)1 << 32);
		errcnt += (int)!testToString<uint64_t>( ++testidx, (uint64_t)1 << 32);
		errcnt += (int)!testToString<int64_t>( ++testidx, (uint64_t)1 << 62);
		errcnt += (int)!testToString<uint64_t>( ++testidx, (uint64_t)1 << 62);
		errcnt += (int)!testToString<int64_t>( ++testidx, +99170709832174LL);
		errcnt += (int)!testToString<uint64_t>( ++testidx, +99170709832174LL);
		errcnt += (int)!testToString<int64_t>( ++testidx, -921391321311323LL);
		errcnt += (int)!testToString<int64_t>( ++testidx, std::numeric_limits<unsigned int>::max());
		errcnt += (int)!testToString<uint64_t>( ++testidx, std::numeric_limits<unsigned int>::max());
		errcnt += (int)!testToString<int64_t>( ++testidx, std::numeric_limits<unsigned int>::min());
		errcnt += (int)!testToString<int64_t>( ++testidx, std::numeric_limits<int>::max());
		errcnt += (int)!testToString<uint64_t>( ++testidx, std::numeric_limits<int>::max());
		errcnt += (int)!testToString<int64_t>( ++testidx, std::numeric_limits<int>::min());
		errcnt += (int)!testToString<int64_t>( ++testidx, std::numeric_limits<int64_t>::min());
		errcnt += (int)!testToString<int64_t>( ++testidx, std::numeric_limits<int64_t>::max());
		errcnt += (int)!testToString<uint64_t>( ++testidx, std::numeric_limits<int64_t>::max());
		errcnt += (int)!testToString<double>( ++testidx, PI);
		errcnt += (int)!testToString<double>( ++testidx, std::numeric_limits<float>::min());
		errcnt += (int)!testToString<double>( ++testidx, std::numeric_limits<float>::max());
		if (errcnt)
		{
			char msgbuf[ 256];
			std::snprintf( msgbuf, sizeof( msgbuf), "%d out of %d tests failed", errcnt, testidx);
			throw std::runtime_error( msgbuf);
		}
		std::cerr << std::endl << "OK done " << testidx << " tests" << std::endl;
		return 0;
	}
	catch (const std::runtime_error& err)
	{
		std::cerr << std::endl << "ERROR " << err.what() << std::endl;
		return -1;
	}
	catch (const std::bad_alloc& )
	{
		std::cerr << std::endl << "ERROR out of memory" << std::endl;
		return -2;
	}
	catch (...)
	{
		std::cerr << std::endl << "EXCEPTION uncaught" << std::endl;
		return -3;
	}
}

