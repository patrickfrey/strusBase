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
#include "strus/base/fileio.hpp"
#include <stdexcept>
#include <iostream>
#include <string>
#include <limits>
#include <cerrno>
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

void errorHandler( void* ctx, int errno_)
{
	std::cerr << "error in handler: " << ::strerror(errno_) << std::endl;
}

int main( int argc, const char** argv)
{
	try
	{
		int nofTexts = (argc <= 1) ? 10 : strus::numstring_conv::touint( argv[1], std::numeric_limits<int>::max());
		if (nofTexts == 0) throw std::runtime_error( strus::string_format( "number of texts (first argument) out of range: %s", argv[1]));

		int textSize = (argc <= 2) ? 100 : strus::numstring_conv::touint( argv[2], std::numeric_limits<int>::max());
		if (textSize == 0) throw std::runtime_error( strus::string_format( "maximum size of texts (second argument) out of range: %s", argv[2]));

		bool useStreamInterface = (argc <= 3) ? false : (0==std::strcmp( "STREAM", argv[3]));

		if (argc > 4) throw std::runtime_error( "too many arguments");

		strus::WriteBufferHandle wbh( errorHandler, 0/*context*/);
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
				if (useStreamInterface)
				{
					FILE* file = wbh.getCStreamHandle();
					if (!file) throw std::runtime_error("failed to get handle as stream");
					nn = ::fwrite( ptr, 1, bytesLeft, file);
				}
				else
				{
					nn = ::write( fh, ptr, bytesLeft);
				}

				if (nn < 0)
				{
					int ec = errno;
					if (ec == 4/*EINTR*/ || ec == 11/*EAGAIN*/) continue;
					if (!ec) throw std::runtime_error( strus::string_format( "error writing on handle (1): %s", ::strerror( ec)));
				}
				int ec = wbh.error();
				if (ec)
				{
					throw std::runtime_error( strus::string_format( "error writing on handle (2): errno %d, %s", ec, ::strerror( ec)));
				}
				ptr += nn;
				bytesLeft -= nn;
			}
		}
		wbh.close();
		std::string result = wbh.fetchContent();

		if (result != expected)
		{
			int ec;
			ec = strus::writeFile( "RES", result);
			if (ec) throw std::runtime_error( strus::string_format( "error writing output file %s: %s", "RES", ::strerror( ec)));
			ec = strus::writeFile( "EXP", expected);
			if (ec) throw std::runtime_error( strus::string_format( "error writing output file %s: %s", "EXP", ::strerror( ec)));

			std::cerr << "RESULT " << result.size()   << " bytes written to file " << "RES" << std::endl;
			std::cerr << "EXPECT " << expected.size() << " bytes written to file " << "EXP" << std::endl;

			throw std::runtime_error( "result not as expected");
		}
		std::cerr << "OK " << (useStreamInterface ? "STREAM":"FH") << " " << nofTexts << " " << textSize << " " << result.size() << std::endl;
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


