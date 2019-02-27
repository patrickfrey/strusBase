/*
 * Copyright (c) 2017 Andreas Baumann
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/fileio.hpp"
#include "strus/base/string_format.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

using namespace strus;
static std::vector<std::string> g_errors;
static bool g_verbose = false;

struct Test
{
	const char* pattern;
	const char* result[31];
};

struct PathDef
{
	const char* part[4];
};

static const PathDef g_pathlist[] = {
	{"A","B","C",0},
	{"A1","B11","C",0},
	{"A1","B11","CC1",0},
	{"AA11","B","A",0},
	{"AA121","121","B",0},
	{"AA121","131","C",0},
	{"AAA121","ABC","C1",0},
	{0,0,0,0}
};

static Test g_tests[] =
{
	{"AA*1/*/A", {
		"AA11/B/A", 0}
	},
	{"A*A*1/1*1/B",{
		"AA121/121/B", 0}
	},
	{"AA?121/*B*/C*",{
		"AAA121/ABC/C1", 0}
	},
	{"A??121/A?C/*",{
		"AAA121/ABC/C1", 0}
	},
	{"A*1\?\?/\?\?\?/*",{
		"AA121/121/B", "AA121/131/C", "AAA121/ABC/C1", 0}
	},
	{"?/B/?",{
		"A/B/C",0}
	},
	{0,{0}}
};

static void createTestFiles( const std::string& rootdir)
{
	PathDef const* pi = g_pathlist;
	for (; pi->part[0]; ++pi)
	{
		std::string path = rootdir;
		char const* const* ti = pi->part;
		for (; *ti; ++ti)
		{
			path = strus::joinFilePath( path, *ti);
			int ec;
			if (ti[1])
			{
				ec = strus::mkdirp( path);
				if (ec) throw std::runtime_error( strus::string_format("failed to create directory %s: %s", path.c_str(), ::strerror(ec)));
			}
			else
			{
				ec = strus::writeFile( path, "");
				if (ec) throw std::runtime_error( strus::string_format("failed to create test file %s: %s", path.c_str(), ::strerror(ec)));
			}
		}
	}
}

static bool executeTest( const std::string& rootdir, const Test& test)
{
	std::vector<std::string> expected;
	std::vector<std::string> result;
	for (int ai=0; test.result[ai]; ++ai) expected.push_back( test.result[ai]);
	std::sort( expected.begin(), expected.end());
	std::string dir( strus::joinFilePath( rootdir, test.pattern));
	std::string::iterator di = dir.begin(), de = dir.end();
	for (; di != de; ++di) if (*di == '/') *di = strus::dirSeparator();
	int ec = strus::expandFilePattern( dir, result);
	if (ec) throw std::runtime_error( strus::string_format( "failed to expand file pattern %s: %s", dir.c_str(), ::strerror(ec)));

	std::vector<std::string>::iterator ri = result.begin(), re = result.end();
	for (; ri != re; ++ri)
	{
		std::size_t prefixsize = rootdir.size();
		if (ri->size() > prefixsize && (*ri)[ prefixsize] == strus::dirSeparator())
		{
			++prefixsize;
		}
		*ri = std::string( ri->c_str() + prefixsize);
	}
	std::sort( result.begin(), result.end());
	bool testFailed = false;
	if (result.size() != expected.size())
	{
		std::cerr << strus::string_format( "size of results differ: results=%d expected=%d", (int)result.size(), (int)expected.size()) << std::endl;
		testFailed = true;
	}
	else
	{
		std::vector<std::string>::const_iterator ai = result.begin(), ae = result.end();
		std::vector<std::string>::iterator bi = expected.begin(), be = expected.end();
		for (di=bi->begin(),de=bi->end(); di != de; ++di) if (*di == '/') *di = strus::dirSeparator();

		for (; ai != ae && bi != be; ++ai,++bi)
		{
			if (*ai != *bi)
			{
				std::cerr << strus::string_format( "result differs: result='%s' expected='%s'", ai->c_str(), bi->c_str()) << std::endl;
				testFailed = true;
			}
		}
	}
	if (testFailed)
	{
		std::cerr << strus::string_format( "test '%s' FAILED", test.pattern) << std::endl;
	}
	else
	{
		std::cerr << strus::string_format( "test '%s' OK", test.pattern) << std::endl;
	}
	return !testFailed;
}



int main( int argc, const char* argv[])
{
	try {
		const char* rootdir = "./";
		if (argc > 1)
		{
			if (argc > 2)
			{
				std::cerr << "Too many arguments" << std::endl;
			}
			if (0==std::strcmp( argv[1], "-V"))
			{
				g_verbose = true;
			}
			else if (0==std::strcmp( argv[1], "-h"))
			{
				std::cout << "Usage: testFilePattern [-V,-h]" << std::endl;
				std::cout << "       Option -V: Verbose output" << std::endl;
				std::cout << "              -h: Print this usage" << std::endl;
			}
			else
			{
				rootdir = argv[1];
			}
		}
		createTestFiles( rootdir);
		int failedCnt = 0;
		int successCnt = 0;
		for (int ti=0; g_tests[ti].pattern; ++ti)
		{
			if (executeTest( rootdir, g_tests[ ti]))
			{
				++successCnt;
			}
			else
			{
				++failedCnt;
			}
		}
		if (failedCnt)
		{
			std::cerr << std::endl << "number of tests failed: " << failedCnt << std::endl;
			std::cerr << std::endl << "number of tests succeeded: " << successCnt << std::endl;
			std::cerr << std::endl << "FAILED" << std::endl;
			return 1;
		}
		else
		{
			std::cerr << std::endl << "OK done" << std::endl;
			return 0;
		}
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

