/*
 * Copyright (c) 2017 Andreas Baumann
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/string_conv.hpp"
#include "strus/base/string_named_format.hpp"
#include "strus/base/string_format.hpp"
#include "strus/base/localErrorBuffer.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <string>

using namespace strus;


static std::string fillNamedFormatString(
		std::vector<strus::NamedFormatString::Chunk>::const_iterator ni,
		std::vector<strus::NamedFormatString::Chunk>::const_iterator ne)
{
	std::string rt;
	for (; ni != ne; ++ni)
	{
		rt.append( ni->prefix());
		if (ni->idx()) rt.append( strus::string_format( "%d", ni->idx()));
	}
	return rt;
}

static void testNamedFormatString()
{
	strus::LocalErrorBuffer errbuf;
	{
		strus::NamedFormatString fmt( "xx{A}y{BB}zz", &errbuf);
		if (errbuf.hasError()) throw std::runtime_error( errbuf.fetchError());
		if (!fmt.assign( "A", 1)) throw std::runtime_error("variable in format string not found");
		if (!fmt.assign( "BB", 2)) throw std::runtime_error("variable in format string not found");
		if (fillNamedFormatString( fmt.begin(), fmt.end()) != "xx1y2zz") throw std::runtime_error( strus::string_format( "named format string test failed on line %d", (int)__LINE__));
	}{
		strus::NamedFormatString fmt( "{ BLAFUNZEL }{ GAGA }", &errbuf);
		if (errbuf.hasError()) throw std::runtime_error( errbuf.fetchError());
		if (!fmt.assign( "BLAFUNZEL", 1)) throw std::runtime_error("variable in format string not found");
		if (!fmt.assign( "GAGA", 2)) throw std::runtime_error("variable in format string not found");
		if (fillNamedFormatString( fmt.begin(), fmt.end()) != "12") throw std::runtime_error( strus::string_format( "named format string test failed on line %d", (int)__LINE__));
	}{
		strus::NamedFormatString fmt( "q{ B }k{ G }z", &errbuf);
		if (errbuf.hasError()) throw std::runtime_error( errbuf.fetchError());
		if (!fmt.assign( "B", 11)) throw std::runtime_error("variable in format string not found");
		if (!fmt.assign( "G", 22)) throw std::runtime_error("variable in format string not found");
		if (fillNamedFormatString( fmt.begin(), fmt.end()) != "q11k22z") throw std::runtime_error( strus::string_format( "named format string test failed on line %d", (int)__LINE__));
	}
}

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
		testidx++; testNamedFormatString();
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

