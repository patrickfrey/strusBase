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
#include <cstring>
#include <string>

using namespace strus;

int main( int argc, const char* argv[])
{
	try {
		int errcnt = 0;
		if (!stringStartsWith( "prefix", "pre")) errcnt++;
		if (stringStartsWith( "prefix", "nono")) errcnt++;
		if (stringStartsWith( "PREFIX", "pre")) errcnt++;
		if (!caseInsensitiveStartsWith( "PREFIX", "pre")) errcnt++;
		if (stringStartsWith( "s", "muchlongerprefixthan")) errcnt++;
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

