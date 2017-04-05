/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/numParser.hpp"
#include "strus/base/string_format.hpp"
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

static void initRand()
{
	time_t nowtime;
	struct tm* now;

	::time( &nowtime);
	now = ::localtime( &nowtime);

	::srand( ((now->tm_year+1) * (now->tm_mon+100) * (now->tm_mday+1)));
}

#define RANDINT(MIN,MAX) ((rand()%(MAX-MIN))+MIN)

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

unsigned int randomValue_uint()
{
	unsigned int digits = RANDINT(0,9);
	return RANDINT( 0, powf( 10, digits));
}

int randomValue_int()
{
	int rt = randomValue_uint();
	return RANDINT( 0, 2) == 0 ? rt:-rt;
}

double randomValue_double()
{
	double rt = randomValue_int();
	rt /= (randomValue_uint()+1);
	return rt;
}

static void checkError( NumParseError err, const char* type)
{
	switch (err)
	{
		case NumParserOk:
			return;
		case NumParserErrNoMem:
			throw std::runtime_error( strus::string_format( "failed to extract %s from configuration string: %s", type, "out of memory"));
		case NumParserErrConversion:
			throw std::runtime_error( strus::string_format( "failed to extract %s from configuration string: %s", type, "conversion error"));
		case NumParserErrOutOfRange:
			throw std::runtime_error( strus::string_format( "failed to extract %s from configuration string: %s", type, "value out of range"));
	}
	throw std::runtime_error( strus::string_format( "failed to extract %s from configuration string: %s", type, "undefined error code"));
}

static void testParseDouble( unsigned int times)
{
	for (unsigned int ti=0; ti<times; ++ti)
	{
		double val = randomValue_double();
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "test FLOAT [" << ti << "]" << " value " << val << std::endl;
#endif
		std::string valstr = strus::string_format( "%.10f", val);
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "\tgot string " << valstr << std::endl;
#endif
		NumParseError err;
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
		NumParseError err;
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
		NumParseError err;
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
		initRand();
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


