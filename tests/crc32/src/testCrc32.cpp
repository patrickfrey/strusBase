/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/crc32.hpp"
#include "strus/base/stdint.h"
#include "private/internationalization.hpp"
#include <stdexcept>
#include <iostream>
#include <cstring>

using namespace strus;

static void crc32Test()
{
	static const char* keys[] = {
		"",
		"a",
		"ab",
		"abc",
		"abcd",
		"abcde",
		"abcdef",
		"abcdefg",
		"abcdefgh",
		"abcdefghi",
		"abcdefghij",
		"abcdefghijk",
		"abcdefghijkl",
		"abcdefghijklm",
		"abcdefghijklmn",
		"abcdefghijklmno",
		"abcdefghijklmnop",
		"abcdefghijklmnopq",
		"abcdefghijklmnopqr",
		"abcdefghijklmnopqrs",
		"abcdefghijklmnopqrst",
		"abcdefghijklmnopqrstu",
		"abcdefghijklmnopqrstuv",
		"abcdefghijklmnopqrstuvw",
		"abcdefghijklmnopqrstuvwx",
		"abcdefghijklmnopqrstuvwxy",
		"abcdefghijklmnopqrstuvwxyz",
		"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
		0
	};
	static uint32_t expected[] =
	{
		0U,
		3904355907U,
		2659403885U,
		891568578U,
		3984772369U,
		2240272485U,
		1267612143U,
		824863398U,
		2934909520U,
		2376698031U,
		964784186U,
		3461812127U,
		4135066404U,
		3723783842U,
		1074632056U,
		1368483807U,
		2486878355U,
		2626835993U,
		150914315U,
		2362763334U,
		442067685U,
		571942307U,
		614063859U,
		955461994U,
		562261492U,
		1093309309U,
		1277644989U,
		2317279385U,
		3692677129U,
	};
	char const** ki = keys;
	for (unsigned int kidx=0; *ki; ++ki,++kidx)
	{
		uint32_t val1 = utils::Crc32::calc( *ki, std::strlen(*ki));
		uint32_t val2 = utils::Crc32::calc( *ki);
		if (val1 != val2) throw strus::runtime_error("crc32 calculation different with and without length parameter in test %u (%u != %u)", kidx, val1, val2);
		if (val1 != expected[kidx]) throw strus::runtime_error( "crc32 calculation not as expected in test %u (%u != %u)", kidx, val1, expected[kidx]);
	}
}

int main( int, const char**)
{
	try
	{
		std::cerr << "executing CRC32 test" << std::endl;
		crc32Test();
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


