/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/error.hpp"
#include "strus/base/string_format.hpp"
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <inttypes.h>

struct TestDef
{
	const char* input;
	const char* output;
	int errcode[4];
};

static TestDef g_tests[] = {
	{"bla bla [#123] bla bla", "bla bla  bla bla", {123,-1,0,0}},
	{"bla bla [123] bla bla", "bla bla [123] bla bla", {-1,0,0,0}},
	{"bla bla [ #99] bla bla", "bla bla [ #99] bla bla", {-1,0,0,0}},
	{"bla bla [#0] bla bla", "bla bla  bla bla", {0,-1,0,0}},
	{"this is an error message with quotes '[#1233231]' bla bla", "this is an error message with quotes '' bla bla", {1233231,-1,0,0}},
	{"this is an error message with 2 error codes [#911] and here comes the second [#912]", "this is an error message with 2 error codes  and here comes the second ", {911,912,-1,0}},
	{"[#1]", "", {1,-1,0,0}},
	{"[#1][#2][#3]", "", {1,2,3,-1}},
	{"[#98765521]x  [#89137103]", "x  ", {98765521,89137103,-1,0}},
	{"[#1a]", "[#1a]", {-1,0,0,0}},
	{0,0,{0,0,0,0}}
};

int main( int, const char**)
{
	try
	{
		TestDef const* testitr = g_tests;
		int testidx = 1;
		for (; testitr->input; ++testitr,++testidx)
		{
			char const* msgitr = testitr->input;
			int ei=0;
			int res_errcode[4] = {0,0,0,0};
			while (ei < 4 && 0 <= (res_errcode[ei++] = strus::errorCodeFromMessage( msgitr))){}
			char output[ 1024];
			if ((int)sizeof(output) <= std::snprintf(output,sizeof(output),"%s",testitr->input)) throw std::runtime_error("local buffer (1K) too small for test input");
			strus::removeErrorCodesFromMessage( output);
			std::cerr << "test " << testidx << strus::string_format("\n\tinp '%s'\n\toc=(%d,%d,%d,%d)\n\tec=(%d,%d,%d,%d)\n\tout '%s'\n\texp '%s'",
					testitr->input,
					res_errcode[0], res_errcode[1], res_errcode[2], res_errcode[3],
					testitr->errcode[0], testitr->errcode[1], testitr->errcode[2], testitr->errcode[3],
					output, testitr->output)
					<< std::endl;
			if (0!=std::memcmp( testitr->errcode, res_errcode, sizeof(res_errcode)))
			{
				throw std::runtime_error("error codes extracted differ from expected");
			}
			if (0!=std::strcmp( testitr->output, output))
			{
				throw std::runtime_error("message cleaned differs from expected");
			}
		}
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


