/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/exec.hpp"
#include "strus/base/numstring.hpp"
#include <stdexcept>
#include <iostream>
#include <cstdio>
#include <cstdlib>

#undef STRUS_LOWLEVEL_DEBUG

#define NOF_TESTS 1000

int main( int argc, const char** argv)
{
	try
	{
		if (argc < 2)
		{
			std::cerr << "Usage: testExec <counter>" << std::endl;
			return -1;
		}
		
		int counter = numstring_conv::toint( argv[1], 2);
		if (counter == 0)
		{
			std::cerr << "OK" << std::endl;
			return 0;
		}
		else
		{
			char counterbuf[ 32];
			std::snprintf( counterbuf, sizeof(counterbuf), "%d", counter-1)
			return 0;
		}
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


