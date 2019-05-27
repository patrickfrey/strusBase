/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/thread.hpp"
#include "strus/base/lockfreeStringMap.hpp"
#include "strus/base/pseudoRandom.hpp"
#include "strus/base/string_format.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <map>

#undef STRUS_LOWLEVEL_DEBUG

bool g_verbose = false;
strus::PseudoRandom g_random;

typedef strus::LockfreeStringMap<int> TestMap;
typedef std::map<std::string,int> ExpectedMap;

class MapIncrement
{
public:
	void operator()( int& dest, int source) const
	{
		dest += source;
	}
};

static void fillLockfreeStringMap( TestMap& testMap, ExpectedMap& expectedMap, int nofElements)
{
	int ni = 0, ne = nofElements;
	for (; ni != ne; ++ni)
	{
		int xx = g_random.get( 0, nofElements);
		char keybuf[ 32];
		std::snprintf( keybuf, sizeof(keybuf), "K%d", xx);
		int value = xx % 131;
		testMap.set( keybuf, value, MapIncrement());
		int oldkey = expectedMap[ keybuf];
		expectedMap[ keybuf] = oldkey + value;

		if (g_verbose) std::cerr << strus::string_format( "update key '%s' value %d + %d = %d", keybuf, oldkey, value, expectedMap[ keybuf]) << std::endl;
	}
}

static void updateLockfreeStringMap( TestMap& testMap, ExpectedMap& expectedMap, int nofElements)
{
	typedef std::vector< std::pair<std::string,int> > UpdateList;
	UpdateList updateList;
	std::set<std::string> keyset;

	int ni = 0, ne = nofElements;
	for (; ni != ne; ++ni)
	{
		int xx = g_random.get( 0, nofElements);
		char keybuf[ 32];
		std::snprintf( keybuf, sizeof(keybuf), "K%d", xx);
		if (keyset.insert( keybuf).second == false) continue;
		int value = xx % 131;
		updateList.push_back( UpdateList::value_type( keybuf, value));

		int oldkey = expectedMap[ keybuf];
		expectedMap[ keybuf] = oldkey + value;

		if (g_verbose) std::cerr << strus::string_format( "update key '%s' value %d + %d = %d", keybuf, oldkey, xx, expectedMap[ keybuf]) << std::endl;
	}
	testMap.set( updateList, MapIncrement());
}

static void checkResultExpected( const TestMap& testMap, const ExpectedMap& expectedMap)
{
	int nofErrors = 0;
	TestMap::const_iterator ti = testMap.begin(), te = testMap.end();
	int nofTestKeys = 0;
	int nofExpectedKeys = 0;

	for (; ti != te; ++ti)
	{
		++nofTestKeys;
		ExpectedMap::const_iterator xi = expectedMap.find( ti->first);
		if (xi == expectedMap.end())
		{
			std::cerr << "test key not found in expected map: '" << ti->first << "'" << std::endl;
			++nofErrors;
		}
		else if (ti->second != xi->second)
		{
			std::cerr << "value of key does not match expected: '" << ti->first << "' -> " << ti->second << " / " << xi->second << std::endl;
			++nofErrors;
		}
	}

	ExpectedMap::const_iterator ei = expectedMap.begin(), ee = expectedMap.end();
	for (; ei != ee; ++ei)
	{
		++nofExpectedKeys;
		int value;
		if (!testMap.get( ei->first.c_str(), value))
		{
			std::cerr << "expected key not found in test map: '" << ei->first << "'" << std::endl;
			++nofErrors;
		}
		else if (value != ei->second)
		{
			std::cerr << "value of key does not match expected in inverse lookup: '" << ei->first << "' -> " << value << " / " << ei->second << std::endl;
			++nofErrors;
		}
	}
	if (nofErrors) throw std::runtime_error( strus::string_format( "%d of differences found comparing %d test keys with %d expected", nofErrors, nofTestKeys, nofExpectedKeys));
}

static int parseNumber( const char* arg)
{
	char const* ai = arg;
	for (; *ai >= '0' && *ai <= '9'; ++ai){}
	if (*ai) throw std::runtime_error("non negative number expected as argument");
	return ::atoi(arg);
}

int main( int argc, const char** argv)
{
	try
	{
		int argi = 1;
		int nofBlocks = 128;
		int nofElements = 10000;
		for (; argc > argi && argv[argi][0] == '-'; ++argi)
		{
			if (0==std::strcmp( argv[argi], "--"))
			{
				++argi;
				break;
			}
			else if (0==std::strcmp( argv[1], "-V"))
			{
				g_verbose = true;
			}
			else if (0==std::strcmp( argv[1], "-h"))
			{
				std::cout << "Usage: testLockfreeStringMap [-V,-h] [<nofblocks>] [<nofelems>]" << std::endl;
				std::cout << "       Option -V: Verbose output" << std::endl;
				std::cout << "              -h: Print this usage" << std::endl;
				std::cout << "       <nofblocks> :Number of blocks in map (default 128)" << std::endl;
				std::cout << "       <nofblocks> :Number of elements in map (default 10000)" << std::endl;
			}
			else
			{
				throw std::runtime_error( "unknown option or argument, use -h for usage");
			}
		}
		if (argi < argc)
		{
			nofBlocks = parseNumber( argv[argi++]);
		}
		if (argi < argc)
		{
			nofElements = parseNumber( argv[argi++]);
		}
		if (argi < argc) throw std::runtime_error( "too many arguments");

		TestMap testMap( nofBlocks);
		ExpectedMap expectedMap;

		fillLockfreeStringMap( testMap, expectedMap, nofElements);
		for (int ii=0; ii<10; ++ii)
		{
			updateLockfreeStringMap( testMap, expectedMap, nofElements < 30 ? 3 : (nofElements/10));
		}
		if (g_verbose)
		{
			std::cerr << "number of blocks: " << nofBlocks << std::endl;
			std::cerr << "number of elements: " << nofElements << std::endl;
			std::cerr << "number of keys inserted: " << expectedMap.size() << std::endl;
		}
		checkResultExpected( testMap, expectedMap);

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


