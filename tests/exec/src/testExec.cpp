/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test program that calls itself with execve_tostring and execv_tostring (strus/base/exec.hpp)
#include "strus/base/exec.hpp"
#include "strus/base/numstring.hpp"
#include "strus/base/fileio.hpp"
#include "strus/base/env.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdlib>

#define STRUS_LOWLEVEL_DEBUG

#define TEST_FILE_STRING "1234567890\n"
#define TEST_RESULT_STRING "ABCDEFGHIJKLMNAOPQRSTUVWXYZ"
#define TEST_FILE "exec.out"
#define TEST_ENV_KEY1 "strus_exec_test_env1"
#define TEST_ENV_KEY2 "strus_exec_test_env2"
#define TEST_ENV_KEY3 "strus_exec_test_env3"
#define TEST_ENV_VAL1 "strus value 1"
#define TEST_ENV_VAL2 "strus value 2"
#define TEST_ENV_VAL3 "strus value 2"

static std::string clearEoln( const std::string& output)
{
	std::string::const_iterator oi = output.begin(), oe = output.end();
	while (oe != oi)
	{
		--oe;
		if (*oe != '\n' && *oe != '\r')
		{
			++oe;
			break;
		}
	}
	return std::string( oi, oe);
}

static bool testEnvVar( const char* var, const char* content)
{
	std::string output;
	int ec = strus::getenv( var, output);
	if (ec)
	{
		std::cerr << "error reading environment variable '" << var << "' in called test program (" << ec << ")" << std::endl;
		return false;
	}
	if (output != content)
	{
		std::cerr << "content of environment variable '" << var << "' not as expected: '" << output << "' != '" << content << "'" << std::endl;
		return false;
	}
	return true;
}

int main( int argc, const char** argv)
{
	try
	{
#ifdef STRUS_LOWLEVEL_DEBUG
		std::ostringstream argstr;
		int ai = 0;
		for (; ai < argc; ai++)
		{
			argstr << " '" << argv[ ai] << "'";
		}
		std::cerr << "START " << argstr.str() << std::endl;
#endif
		int counter = (argc < 2) ? 2 : strus::numstring_conv::toint( argv[1], 2);
		if (counter == 0)
		{
			int ec = strus::writeFile( TEST_FILE, TEST_FILE_STRING);
			if (ec)
			{
				std::cerr << "error writing file '" << TEST_FILE << "' (" << ec << ")" << std::endl;
				return ec;
			}
			std::cout << TEST_RESULT_STRING << std::endl;
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cerr << "DONE " << argstr.str() << std::endl;
#endif
			return 0;
		}
		else if (counter == 1)
		{
			char const* call_argv[3] = {argv[0], "0", NULL};
			std::string output;
			int ec = strus::execv_tostring( argv[0], call_argv, output);
			if (ec)
			{
				std::cerr << "error calling program (execv) '" << argv[0] << "' (" << ec << ")" << std::endl;
				return ec;
			}
			std::string result = clearEoln( output);
			std::string expect = TEST_RESULT_STRING;
			std::cerr << "RESULT [" << result << "] size " << result.size() << std::endl;
			std::cerr << "EXPECT [" << expect << "] size " << expect.size() << std::endl;
			if (result != expect)
			{
				std::cerr << "result of called program (execv) does not match: '" << result << "' != '" << expect << "'" << std::endl;
				return -1;
			}
			output.clear();
			ec = strus::readFile( TEST_FILE, output);
			if (ec)
			{
				std::cerr << "error reading file '" << TEST_FILE << "' (" << ec << ")" << std::endl;
				return ec;
			}
			if (output != TEST_FILE_STRING)
			{
				std::cerr << "result of called program (execv) does not match: '" << clearEoln( output) << "\\n' != '" << clearEoln( TEST_FILE_STRING) << "\\n'" << std::endl;
				return -1;
			}
			if (!testEnvVar( TEST_ENV_KEY1, TEST_ENV_VAL1)) return -1;
			if (!testEnvVar( TEST_ENV_KEY2, TEST_ENV_VAL2)) return -1;
			if (!testEnvVar( TEST_ENV_KEY3, TEST_ENV_VAL3)) return -1;
			std::cout << TEST_RESULT_STRING;
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cerr << "DONE " << argstr.str() << std::endl;
#endif
			return 0;
		}
		else
		{
			std::map<std::string,std::string> call_env;
			call_env[ TEST_ENV_KEY1] = TEST_ENV_VAL1;
			call_env[ TEST_ENV_KEY2] = TEST_ENV_VAL2;
			call_env[ TEST_ENV_KEY3] = TEST_ENV_VAL3;

			char const* call_argv[3] = {argv[0], "1", NULL};
			std::string output;
			int ec = strus::execve_tostring( argv[0], call_argv, call_env, output);
			if (ec)
			{
				std::cerr << "error calling program (execve) '" << argv[0] << "' (" << ec << ")" << std::endl;
				return ec;
			}
			std::string result = clearEoln( output);
			if (result != TEST_RESULT_STRING)
			{
				std::cerr << "result of called program (execv) does not match: '" << result << "' != '" << TEST_RESULT_STRING << "'" << std::endl;
				return -1;
			}
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cerr << "DONE " << argstr.str() << std::endl;
#endif
			std::cerr << "OK" << std::endl;
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


