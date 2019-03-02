/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/regex.hpp"
#include "strus/base/localErrorBuffer.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>

using namespace strus;

#define STRUS_LOWLEVEL_DEBUG

static LocalErrorBuffer g_errhnd;

bool testRegexSearch( const char* regex, const char* input, int pos, int len)
{
	RegexSearch search( regex, 0, &g_errhnd);
	if (g_errhnd.hasError())
	{
		std::cerr << "ERROR " << g_errhnd.fetchError() << std::endl;
		return false;
	}
	RegexSearch::Match mt = search.find( input, input + std::strlen(input));
	if (!mt.valid())
	{
		if (g_errhnd.hasError())
		{
			std::cerr << "ERROR " << g_errhnd.fetchError() << std::endl;
			return false;
		}
	}
	if (pos != mt.pos || len != mt.len)
	{
		std::cerr << "ERROR " << "match not as expected ( " << pos << "==" << mt.pos << " && " << len << "==" << mt.len << " )" << std::endl;
		return false;
	}
	return true;
}

bool testRegexFindStart( const char* regex, const char* input, int len)
{
	RegexSearch search( regex, 0, &g_errhnd);
	if (g_errhnd.hasError())
	{
		std::cerr << "ERROR " << g_errhnd.fetchError() << std::endl;
		return false;
	}
	int explen = search.find_start( input, input + std::strlen(input));
	if (explen < 0)
	{
		if (g_errhnd.hasError())
		{
			std::cerr << "ERROR " << g_errhnd.fetchError() << std::endl;
			return false;
		}
	}
	if (len != explen)
	{
		std::cerr << "ERROR " << "start match not as expected ( " << len << "==" << explen << " )" << std::endl;
		return false;
	}
	return true;
}

const char* boolTostring( bool val)
{
	return val?"true":"false";
}

bool testRegexMatch( const char* regex, const char* input, int len, bool expect)
{
	RegexSearch search( regex, 0, &g_errhnd);
	if (g_errhnd.hasError())
	{
		std::cerr << "ERROR " << g_errhnd.fetchError() << std::endl;
		return false;
	}
	bool result = search.match( input, std::strlen(input));
	if (g_errhnd.hasError())
	{
		std::cerr << "ERROR " << g_errhnd.fetchError() << std::endl;
		return false;
	}
	if (result != expect)
	{
		std::cerr << "ERROR " << "match not as expected ( " << boolTostring(result) << "==" << boolTostring(expect) << " )" << std::endl;
		return false;
	}
	return true;
}

int main( int argc, const char* argv[])
{
	if (argc >= 2 && (std::strcmp( argv[1], "-h") == 0 || std::strcmp( argv[1], "--help") == 0))
	{
		std::cerr << "testRegex" << std::endl;
		return 0;
	}
	try
	{
		int errcnt = 0;
		int testcnt = 0;
		++testcnt; if (!testRegexSearch( "[A-Z]+", "abcABZgfh", 3, 3)) ++errcnt;
		++testcnt; if (!testRegexSearch( "[A-Z0-9]+", "abcABZ9gfh", 3, 4)) ++errcnt;
		++testcnt; if (!testRegexSearch( "[0-9]+", "abcABZgfh", -1, 0)) ++errcnt;
		++testcnt; if (!testRegexFindStart( "[A-Z]+", "ABZgfh", 3)) ++errcnt;
		++testcnt; if (!testRegexFindStart( "[A-Z]+", "ABZFgfh", 4)) ++errcnt;
		++testcnt; if (!testRegexFindStart( "[A-Z]+", "aBZFgfh", -1)) ++errcnt;
		++testcnt; if (!testRegexMatch("[0-9]{1,3}", "567", 3, true)) ++errcnt;
		++testcnt; if (!testRegexMatch("[0-9]{2,3}", "5", 1, false)) ++errcnt;
		++testcnt; if (!testRegexMatch("[0-9]{1,3}", "A567", 4, false)) ++errcnt;
		++testcnt; if (!testRegexMatch("[0-9]{1,3}", "567A", 4, false)) ++errcnt;
		++testcnt; if (!testRegexMatch("[0-9]{1,3}", "5674", 4, false)) ++errcnt;
		++testcnt; if (!testRegexMatch("[0-9]{1,3}", "567", 3, true)) ++errcnt;
		++testcnt; if (!testRegexMatch("x|xx|xxx|xxxx", "xxxx", 4, true)) ++errcnt;
		if (errcnt)
		{
			char msgbuf[ 256];
			std::snprintf( msgbuf, sizeof( msgbuf), "%d out of %d tests failed", errcnt, testcnt);
			throw std::runtime_error( msgbuf);
		}
		std::cerr << std::endl << "OK done " << testcnt << " tests" << std::endl;
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

