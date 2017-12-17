/*
 * Copyright (c) 2017 Andreas Baumann
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/string_conv.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <string>

using namespace strus;

int main( int argc, const char* argv[])
{
	try {
		int errcnt = 0;
		int testidx = 0;
		testidx++; if (!stringStartsWith( "prefix", "pre")) errcnt++;
		testidx++; if (stringStartsWith( "prefix", "nono")) errcnt++;
		testidx++; if (stringStartsWith( "xxprefix", "pre")) errcnt++;
		testidx++; if (stringStartsWith( "PREFIX", "pre")) errcnt++;
		testidx++; if (!caseInsensitiveStartsWith( "PREFIX", "pre")) errcnt++;
		testidx++; if (stringStartsWith( "s", "muchlongerprefixthan")) errcnt++;
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

