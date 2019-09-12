/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test program that calls itself with execve_tostring and execv_tostring (strus/base/exec.hpp)
#include "strus/base/filehandle.hpp"
#include "strus/base/pseudoRandom.hpp"
#include "strus/base/utf8.hpp"
#include "strus/base/numstring.hpp"
#include "strus/base/string_format.hpp"
#include <stdexcept>
#include <iostream>
#include <string>
#include <unistd.h>

static strus::PseudoRandom g_random;

static std::string getCharacter( int code)
{
	char chrbuf[ 16];
	std::size_t chrlen = strus::utf8encode( chrbuf, code);
	return std::string( chrbuf, chrlen);
}

static std::string randomCharacter()
{
	int code = g_random.get( 1, 32) == 1 ? '\n' : g_random.get( 1, 5) > 1 ? g_random.get( 1, 128) : g_random.get( 1, 1 << g_random.get( 1,16));
	return getCharacter( code);
}

static std::string randomText( std::size_t maxsize)
{
	std::string rt;
	std::size_t si = 0, se = g_random.get( 1, g_random.get( 1, maxsize)+1);
	for (; si != se; ++si)
	{
		rt.append( randomCharacter());
	}
	return rt;
}

int main( int argc, const char** argv)
{
	try
	{
		int nofTexts = (argc <= 1) ? 10 : strus::numstring_conv::touint( argv[1], std::numeric_limits<int>::max());
		if (nofTexts == 0) throw std::runtime_error( strus::string_format( "number of texts (first argument) out of range: %s", argv[1]));

		int textSize = (argc <= 2) ? 100 : strus::numstring_conv::touint( argv[2], std::numeric_limits<int>::max());
		if (textSize == 0) throw std::runtime_error( strus::string_format( "maximum size of texts (second argument) out of range: %s", argv[2]));

		if (argc > 3) throw std::runtime_error( "too many arguments");

		strus::WriteBufferHandle wbh;
		std::string expected;

		int fh = wbh.fileHandle();

		int wi = 0, we = nofTexts;
		for (; wi != we; ++wi)
		{
			std::string text = randomText( textSize);
			expected.append( text);

			ssize_t bytesLeft = text.size();
			char const* ptr = text.c_str();
			ssize_t nn = 0;
			while (nn < bytesLeft)
			{
				nn = ::write( fh, ptr, bytesLeft);

				if (nn < 0)
				{
					int ec = errno;
					if (ec == 4/*EINTR*/) continue;

					throw std::runtime_error( strus::string_format( "error writing on handle: %s", ::strerror( ec)));
				}
				if (wbh.error())
				{
					throw std::runtime_error( strus::string_format( "error writing on handle: %s", ::strerror( wbh.error())));
				}
				ptr += nn;
				bytesLeft -= nn;
			}
		}
		wbh.done();
		std::string result = wbh.fetchContent();

		if (result != expected)
		{
			std::cerr << "RESULT:" << std::endl;
			std::cerr << result << std::endl << std::endl;
			std::cerr << "EXPECTED:" << std::endl;
			std::cerr << expected << std::endl << std::endl;

			throw std::runtime_error( "result not as expected");
		}
		std::cerr << "OK " << nofTexts << " " << textSize << " " << result.size() << std::endl;
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


