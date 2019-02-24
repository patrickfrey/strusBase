/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/numstring.hpp"
#include "strus/base/string_format.hpp"
#include "strus/base/pseudoRandom.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <limits>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <inttypes.h>

#define STRUS_LOWLEVEL_DEBUG

static strus::PseudoRandom g_random;

using namespace strus;

static unsigned int powf( unsigned int base, unsigned int n)
{
	unsigned int rt = 1;
	while (n-- != 0)
	{
		rt *= base;
	}
	return rt;
}

static unsigned int randomValue_uint()
{
	unsigned int digits = g_random.get(0,9);
	return g_random.get( 0, powf( 10, digits));
}

static int randomValue_int()
{
	int rt = randomValue_uint();
	return g_random.get( 0, 2) == 0 ? rt:-rt;
}

static double randomValue_double()
{
	double rt = randomValue_int();
	rt /= (randomValue_uint()+1);
	return rt;
}

static void checkError( NumParseError err, const char* type)
{
	switch (err)
	{
		case NumParseOk:
			return;
		case NumParseErrNoMem:
			throw std::runtime_error( strus::string_format( "failed to convert string to %s: %s", type, "out of memory"));
		case NumParseErrConversion:
			throw std::runtime_error( strus::string_format( "failed to convert string to %s: %s", type, "conversion error"));
		case NumParseErrOutOfRange:
			throw std::runtime_error( strus::string_format( "failed to convert string to %s: %s", type, "value out of range"));
		case NumParseErrInvalidArg:
			throw std::runtime_error( strus::string_format( "failed to convert string to %s: %s", type, "invalid argument"));
	}
	throw std::runtime_error( strus::string_format( "failed to convert string to %s: %s", type, "undefined error code"));
}

static void testParseDouble( unsigned int times)
{
	for (unsigned int ti=0; ti<times; ++ti)
	{
		double val = randomValue_double();
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "test FLOAT [" << ti << "]" << " value " << val << std::endl;
#endif
		std::string valstr = strus::string_format( "%.12f", val);
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "\tgot string " << valstr << std::endl;
#endif
		NumParseError err = NumParseOk;
		double val_test = strus::doubleFromString( valstr, err);
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "\tgot result (" << (unsigned int)err << ") " << val_test << std::endl;
#endif
		checkError( err, "FLOAT");

		double diff = fabs( val_test - val);
		double tolerance = std::numeric_limits<float>::epsilon();
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "\tgot diff " << diff << " (tolerance " << tolerance << ")" << std::endl;
#endif
		if (diff > tolerance)
		{
			throw std::runtime_error( std::string("string conversion test failed for type ") + "double");
		}
	}
}

static void testParseUInt( unsigned int times)
{
	for (unsigned int ti=0; ti<times; ++ti)
	{
		unsigned int val = randomValue_uint();
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "test UINT [" << ti << "]" << " value " << val << std::endl;
#endif
		std::string valstr = strus::string_format( "%u", val);
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "\tgot string " << valstr << std::endl;
#endif
		NumParseError err = NumParseOk;
		unsigned int val_test = strus::uintFromString( valstr, std::numeric_limits<unsigned int>::max(), err);
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "\tgot result (" << (unsigned int)err << ") " << val_test << std::endl;
#endif
		checkError( err, "UINT");

#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "\tgot diff " << (int)(val - val_test) << std::endl;
#endif
		if (val != val_test)
		{
			throw std::runtime_error( std::string("string conversion test failed for type ") + "UINT");
		}
	}
}

static void testParseInt( unsigned int times)
{
	for (unsigned int ti=0; ti<times; ++ti)
	{
		int val = randomValue_int();
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "test INT [" << ti << "]" << " value " << val << std::endl;
#endif
		std::string valstr = strus::string_format( "%d", val);
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "\tgot string " << valstr << std::endl;
#endif
		NumParseError err = NumParseOk;
		int val_test = strus::intFromString( valstr, (unsigned int)std::numeric_limits<int>::max() +1, err);
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "\tgot result (" << (unsigned int)err << ") " << val_test << std::endl;
#endif
		checkError( err, "INT");

#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "\tgot diff " << (int)(val - val_test) << std::endl;
#endif
		if (val != val_test)
		{
			throw std::runtime_error( std::string("string conversion test failed for type ") + "INT");
		}
	}
}

#define NOF_TESTS 1000

int main( int, const char**)
{
	try
	{
		std::cerr << "executing " << NOF_TESTS << " tests num parser with double" << std::endl;
		testParseDouble( NOF_TESTS);
		std::cerr << "OK" << std::endl;

		std::cerr << "executing " << NOF_TESTS << " tests num parser with uint" << std::endl;
		testParseUInt( NOF_TESTS);
		std::cerr << "OK" << std::endl;

		std::cerr << "executing " << NOF_TESTS << " tests num parser with int" << std::endl;
		testParseInt( NOF_TESTS);
		std::cerr << "OK" << std::endl;
		return 0;
	}
	catch (const std::bad_alloc& err)
	{
		std::cerr << "ERROR " << err.what() << std::endl;
	}
	catch (const std::exception& err)
	{
		std::cerr << "ERROR " << err.what() << std::endl;
	}
	return -1;
}


