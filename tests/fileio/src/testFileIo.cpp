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

using namespace strus;
static std::vector<std::string> g_errors;
static bool g_verbose = false;

static void test_resolveUpdirReferences()
{
	static const char* testname = "resolveUpdirReferences";
	struct Test
	{
		const char* in;
		const char* out;
	};
	static const Test testar[] = {
		{"/home/bruno", "/home/bruno"},
		{"/home/bruno/", "/home/bruno/"},
		{"/home/hugo/../bruno", "/home/bruno"},
		{"//home/hugo/../bruno", "/home/bruno"},
		{"/home/hugo/...//bruno", "/home/hugo/.../bruno"},
		{"/home/bruno//", "/home/bruno/"},
		{"//home//hugo/../bruno", "/home/bruno"},
		{"/home/hugo/./bruno", "/home/hugo/bruno"},
		{"./home/hugo/../bruno", "./home/bruno"},
		{"..home/hugo/../bruno", "..home/bruno"},
		{NULL,NULL}
	};
	Test const* ti = testar;
	for (; ti->in; ++ti)
	{
		std::string pt( ti->in);
		int ec = resolveUpdirReferences( pt);
		if (ec) g_errors.push_back( string_format( "test %s ('%s','%s') failed: %s", testname, ti->in, ti->out, ::strerror(ec)));
		if (pt != ti->out) g_errors.push_back( string_format( "test %s ('%s','%s') failed, output differs: '%s'", testname, ti->in, ti->out, pt.c_str()));
		if (g_verbose) std::cerr << string_format( "executing test %s ('%s','%s') => '%s'", testname, ti->in, ti->out, pt.c_str()) << std::endl;
	}
}

static void test_mkdirp()
{
	std::string testdir;
	int ec;
	(void)strus::removeDir( "./fileio_test/A/B/C");
	(void)strus::removeDir( "./fileio_test/A/B");
	(void)strus::removeDir( "./fileio_test/A");
	(void)strus::removeDir( "./fileio_test");
	ec = strus::mkdirp( "./fileio_test");
	if (ec || !strus::isDir( "./fileio_test"))
	{
		if (ec) std::cerr << "file error: " << ::strerror(ec);
		throw std::runtime_error( "failed to create test direcory with mkdirp");
	}
	ec = strus::mkdirp( "./fileio_test/A/B/C", testdir);
	if (ec || !strus::isDir( "./fileio_test/A/B/C") || testdir != "./fileio_test/A")
	{
		if (ec) std::cerr << "file error: " << ::strerror(ec);
		throw std::runtime_error( "mkdirp with out parameter for rollback failed");
	}
}

static void test_replaceFileExtension()
{
	if (strus::replaceFileExtension( "bla.txt", ".ext") != "bla.ext"
	||  strus::replaceFileExtension( "bla.txt", "ext") != "bla.ext"
	||  strus::replaceFileExtension( "gaga/blu.", ".tx") != "gaga/blu.tx"
	||  strus::replaceFileExtension( "gaga/blu", ".tx") != "gaga/blu.tx"
	||  strus::replaceFileExtension( "", ".tx") != ".tx")
	{
		throw std::runtime_error( "replaceFileExtension failed");
	}
}

int main( int argc, const char* argv[])
{
	try {
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
				throw std::runtime_error( "unknown option or argument, use -h");
			}
		}
		test_resolveUpdirReferences();
		test_mkdirp();
		test_replaceFileExtension();

		if (!g_errors.empty())
		{
			std::vector<std::string>::const_iterator ei = g_errors.begin(), ee = g_errors.end();
			for (; ei != ee; ++ei)
			{
				std::cerr << *ei << std::endl;
			}
			throw std::runtime_error( string_format( "%d tests failed", (int)g_errors.size()));
		}
		std::cerr << std::endl << "OK done" << std::endl;
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

