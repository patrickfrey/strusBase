/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/utf8.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <limits>
#include <ctime>
#include "strus/base/stdint.h"

static bool g_verbose = false;

static void utf8encodeDecodeTestCharacter( int32_t chr)
{
	char buf[ 16];
	std::size_t chrsize = strus::utf8encode( buf, chr);
	int32_t oth = strus::utf8decode( buf, chrsize);
	if (chr != oth)
	{
		char errbuf[ 1024];
		std::snprintf( errbuf, sizeof(errbuf), "Failed to encode/decode UTF8 character %d", (int)chr);
		throw std::runtime_error( errbuf);
	}
	else if (g_verbose)
	{
		printf( "Checking encode/decode of Unicode character %x\n", chr);
	}
}

static void utf8encodeDecodeTestRange( int32_t start, int32_t end, int32_t step)
{
	int32_t itr = start;
	for (;itr < end; itr += step)
	{
		utf8encodeDecodeTestCharacter( itr);
	}
}

static void utf8encodeDecodeTest()
{
	enum {MaxCharacter=0x7FFF};
	utf8encodeDecodeTestCharacter( 0);
	utf8encodeDecodeTestCharacter( MaxCharacter);
	utf8encodeDecodeTestRange( 0, 512, 7);
	utf8encodeDecodeTestRange( 0x200, 0x800, 51);
	utf8encodeDecodeTestRange( 0x800, 0x4000, 321);
	utf8encodeDecodeTestRange( 0x4000, 0x8000, 713);
	utf8encodeDecodeTestRange( 0x8000, MaxCharacter, 1011);
}

int main( int argc, const char** argv)
{
	try
	{
		if (argc > 1)
		{
			if (0==std::strcmp( argv[1],"-V"))
			{
				g_verbose = true;
			}
			if (0==std::strcmp( argv[1],"-h"))
			{
				std::cerr << argv[0] << " [-V][-h]" << std::endl;
			}
		}
		utf8encodeDecodeTest();
		std::cerr << "OK" << std::endl;
		return 0;
	}
	catch (const std::exception& err)
	{
		std::cerr << "EXCEPTION " << err.what() << std::endl;
	}
	return -1;
}


