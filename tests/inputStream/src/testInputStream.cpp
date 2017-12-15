/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/inputStream.hpp"
#include "strus/base/stdint.h"
#include "strus/base/fileio.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <limits>
#include <ctime>
#include <cstdlib>
#include <cstring>

#undef STRUS_LOWLEVEL_DEBUG

static void initRand()
{
	time_t nowtime;
	struct tm* now;

	::time( &nowtime);
	now = ::localtime( &nowtime);

	::srand( ((now->tm_year+1) * (now->tm_mon+100) * (now->tm_mday+1)));
}

#define RANDINT(MIN,MAX) ((rand()%(MAX-MIN))+MIN)

using namespace strus;

static std::string randomContent( unsigned int size)
{
	static const char* alphabet = "\n\tabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::size_t alphabetSize = std::strlen( alphabet);
	std::string rt;
	unsigned int si = 0, se = size;
	for (; si != se; ++si)
	{
		rt.push_back( alphabet[ RANDINT(0,alphabetSize)]);
	}
	return rt;
}

static void test( const std::string& testfilename, unsigned int docsize)
{
	std::string content( randomContent( docsize));
#ifdef STRUS_LOWLEVEL_DEBUG
	std::cout << "test content:" << std::endl << content << std::endl;
#endif
	unsigned int ec = writeFile( testfilename, content);
	if (ec) throw std::runtime_error( ::strerror(ec));
	InputStream inputstream( testfilename);
	std::string readahead;
	std::string bytesread;
	char buf[ 1024];
	std::size_t nn;
	const char* line;
	bool eof = false;
	while (!eof)
	{
		unsigned int what = RANDINT(0,3);
		switch (what)
		{
			case 0: //READ
				nn = inputstream.read( buf, RANDINT(1,sizeof(buf)));
				bytesread.append( buf, nn);
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "[read][" << std::string(buf,nn) << "]" << std::endl;
#endif
				if (nn == 0)
				{
					unsigned int error = inputstream.error();
					if (!error)
					{
						eof = true;
					}
					else if (error != 22/*EINVAL*/)
					{
						throw std::runtime_error(::strerror(error));
					}
				}
				else
				{
					unsigned int ra = nn > readahead.size() ? readahead.size() : nn;
					if (std::memcmp( buf, readahead.c_str(), ra) != 0)
					{
						throw std::runtime_error("failed read ahead (read)");
					}
					readahead.clear();
				}
				break;
			case 1: //READ AHEAD:
				nn = RANDINT( 1, sizeof(buf));
				nn = inputstream.readAhead( buf, nn);
				readahead = std::string( buf, nn);
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "[read ahead][" << readahead << "]" << std::endl;
#endif
				if (nn == 0)
				{
					unsigned int error = inputstream.error();
					if (!error)
					{
						eof = true;
					}
					else if (error != EINVAL)
					{
						throw std::runtime_error(::strerror(error));
					}
				}
				break;
			case 2: //READ LINE:
				nn = RANDINT( 1, sizeof(buf));
				line = inputstream.readLine( buf, nn, true);
				if (line)
				{
#ifdef STRUS_LOWLEVEL_DEBUG
					std::cout << "[read line][" << line << "]" << std::endl;
#endif
					bytesread.append( line);
					bytesread.push_back( '\n');

					nn = std::strlen( line);
					unsigned int ra = nn > readahead.size() ? readahead.size() : nn;
					if (std::memcmp( buf, readahead.c_str(), ra) != 0)
					{
						throw std::runtime_error("failed read ahead (read line)");
					}
					readahead.clear();
				}
				else
				{
					unsigned int error = inputstream.error();
					if (!error)
					{
						eof = true;
					}
					else if (error != EINVAL)
					{
						throw std::runtime_error(::strerror(error));
					}
				}
				break;
		}
	}
#ifdef STRUS_LOWLEVEL_DEBUG
	std::cout << "[eof]" << std::endl;
	if (bytesread != content)
	{
		std::size_t si=0, se=std::min( bytesread.size(), content.size());
		for (; si != se && bytesread[si] == content[si]; ++si){}
		std::ostringstream msg;
		msg << "bytes read and input do not match at position " << si << std::endl;
		throw std::runtime_error( msg.str());
	}
#endif
}

int main( int argc, const char** argv)
{
	try
	{
		initRand();
		std::string testFileName = (argc < 2) ? "random.txt" : argv[1];
		unsigned int times = (argc < 3) ? 10 : atoi(argv[2]);

		unsigned int ti = 0, te = times;
		for (; ti != te; ++ti)
		{
			test( testFileName, ti * 2000 + 1);
		}
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


