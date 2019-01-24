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

static int createTestFiles( const std::string& rootdir, int level=2)
{
	int rt = 0;
	static char const* prefix[] = {"","A","AA",0};
	static char const* suffix[] = {"","B","BB",0};
	static char const* mid[] = {"","1","11","111","211","121","112",0};
	char const** pi = prefix;
	for (; *pi; ++pi)
	{
		char const** si = suffix;
		for (; *si; ++si)
		{
			char const** mi = mid;
			for (; *mi; ++mi)
			{
				std::string nam = std::string(*pi) + *mi + *si;
				if (nam.empty()) continue;
				std::string path( strus::joinFilePath( rootdir, nam));
				if (level > 0)
				{
					int ec = strus::mkdirp( path);
					if (ec) throw std::runtime_error( strus::string_format("failed to create directory %s: %s", path.c_str(), ::strerror(ec)));

					rt += createTestFiles( path, level-1);
				}
				else
				{
					int ec = strus::writeFile( path, "");
					if (ec) throw std::runtime_error( strus::string_format("failed to create test file %s: %s", path.c_str(), ::strerror(ec)));
					rt += 1;
				}
			}
		}
	}
	return rt;
}

struct Test
{
	const char* pattern;
	const char* result[31];
};

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

static Test g_tests[] =
{
	{"AA*1/111*/A",{
		"AA111/111/A","AA11/111B/A","AA1/111BB/A","AA211/111/A",
		"AA111/111B/A","AA11/111BB/A","AA121/111/A","AA211/111B/A",
		"AA111/111BB/A","AA1/111/A","AA121/111B/A","AA211/111BB/A",
		"AA11/111/A","AA1/111B/A","AA121/111BB/A",0}
	},
	{"A*A*1/111*/A",{
		"AA111/111/A","AA11/111B/A","AA1/111BB/A","AA211/111/A",
		"AA111/111B/A","AA11/111BB/A","AA121/111/A","AA211/111B/A",
		"AA111/111BB/A","AA1/111/A","AA121/111B/A","AA211/111BB/A",
		"AA11/111/A","AA1/111B/A","AA121/111BB/A",0}
	},
	{"A?11/111*/A",{
		"A111/111/A","A211/111/A","AA11/111/A",
		"A111/111B/A","A211/111B/A","AA11/111B/A",
		"A111/111BB/A","A211/111BB/A","AA11/111BB/A",0}
	},
	{"A??1/111?/A",{
		"A111/111B/A","A121/111B/A","A211/111B/A","AA11/111B/A",0}
	},
	{"A?1?1/11*B?/A",{
		"AA111/111BB/A","AA111/11BB/A","AA121/112BB/A","AA111/112BB/A","AA121/111BB/A","AA121/11BB/A",0}
	},
	{"?/A/?",{
		"1/A/1","1/A/B","A/A/A","B/A/1","B/A/B",
		"1/A/A","A/A/1","A/A/B","B/A/A",0}
	},
	{"AA1*/A/1",{
		"AA111/A/1","AA112/A/1","AA11/A/1","AA121/A/1","AA1/A/1",
		"AA111B/A/1","AA112B/A/1","AA11B/A/1","AA121B/A/1","AA1B/A/1",
		"AA111BB/A/1","AA112BB/A/1","AA11BB/A/1","AA121BB/A/1","AA1BB/A/1",0}
	},
	{"*1BB/A/1",{
		"111BB/A/1","1BB/A/1","A11BB/A/1","A211BB/A/1","AA121BB/A/1",
		"11BB/A/1","211BB/A/1","A121BB/A/1","AA111BB/A/1","AA1BB/A/1",
		"121BB/A/1","A111BB/A/1","A1BB/A/1","AA11BB/A/1","AA211BB/A/1",0}
	},
	{"1/1/2*",{
		"1/1/211","1/1/211B","1/1/211BB",0}
	},
	{"1/1/1?",{
		"1/1/11","1/1/1B",0}
	},
	{"AA/11/AA",{
		 "AA/11/AA",0}
	},
	{0,{0}}
};


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
				std::cout << "Usage: testFileIo [-V,-h]" << std::endl;
				std::cout << "       Option -V: Verbose output" << std::endl;
				std::cout << "              -h: Print this usage" << std::endl;
			}
			else
			{
				rootdir = argv[1];
			}
		}
		int nofFiles = createTestFiles( rootdir);
		std::cerr << strus::string_format( "created %d test files", nofFiles) << std::endl;
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

