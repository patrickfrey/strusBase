/*
 * Copyright (c) 2017 Andreas Baumann
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/string_conv.hpp"
#include "strus/base/string_format.hpp"
#include "strus/timeStamp.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <string>

using namespace strus;

static void testTimeStampExample( const char* example, int year, int mon, int day, int hrs, int min, int sec, int msec)
{
	TimeStamp tms = timeStampFromString( example);
	if (tms == -1) throw std::runtime_error("Failed to parse timestamp string");
	TimeStampString str = timeStampToString( tms);
	if (0!=std::strcmp(str.str,example)) throw std::runtime_error("Failed to parse timestamp string and convert it back to a timestamp");
	TimeStamp ymd = timeStampFromdate( year, mon, day, hrs, min, sec, msec);
	if (ymd == -1) throw std::runtime_error("Failed to initialize timestamp from its date structure members");
	if (ymd != tms) throw std::runtime_error("Timestamps initialized in 2 different ways differ");
}

static void testTimeStampGen( int steps)
{
	TimeStamp start = timeStampFromdate( 1970, 1, 1, 0, 0, 0, 0);
	if (start != 0) throw std::runtime_error("Timestamp for 1.1.1970 00:00:00 is not 0");
	TimeStamp end = timeStampFromdate( 2030, 1, 1, 0, 0, 0, 0);
	int64_t step = (end-start) / steps;
	TimeStamp itr = start;
	int count = 1;
	for (; itr < end; itr += step, count += 1)
	{
		TimeStampString str = timeStampToString( itr);
		TimeStamp tms = timeStampFromString( str.str);
		if (tms != itr)
		{
			char buf[1024];
			std::snprintf( buf, sizeof(buf), "Test timestamp gen failed in iteration %d\n", count);
			throw std::runtime_error( buf);
		}
	}
}

static void testCurrentTimeStamp()
{
	TimeStamp cur = getCurrentTimeStamp();
	TimeStampString str = timeStampToString( cur);
	fprintf( stderr, "Current date/time = %s\n", str.str);
	if (!str.str[0]) throw std::runtime_error("Failed to map current timestamp to a string");
}

static void testTimeStamp()
{
	testTimeStampExample( "19990107123121671", 1999,  1, 07, 12, 31, 21, 671);
	testTimeStampExample( "19721231235959999", 1972, 12, 31, 23, 59, 59, 999);
	testTimeStampExample( "19721231010159999", 1972, 12, 31,  1,  1, 59, 999);
	testTimeStampExample( "20210906051359123", 2021,  9,  6,  5, 13, 59, 123);
	testTimeStampExample( "20010906011359000", 2001,  9,  6,  1, 13, 59, 000);
}

int main( int argc, const char* argv[])
{
	try {
		testCurrentTimeStamp();
		testTimeStampGen( 10000);
		testTimeStamp();
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

