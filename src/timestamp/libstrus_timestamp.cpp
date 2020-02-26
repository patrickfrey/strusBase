/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief timestamp methods implementation
/// \file libstrus_timestamp.cpp
#include "strus/timeStamp.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/atomic.hpp"
#include "strus/base/string_conv.hpp"
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdlib>

#define _TXT(str) str

using namespace strus;

static bool onlyDigits( char const* ti)
{
	for (;*ti >= '0' && *ti <= '9'; ++ti){}
	return !*ti;
}

static TimeStamp parseTimeStamp( const char* timestampstr, ErrorCode& errcode)
{
	char const* ti = timestampstr;
	struct tm date;

	char YY_buf[ 5];
	std::memcpy( YY_buf, ti, 4); YY_buf[4] = 0; ti += 4;
	char mm_buf[ 3];
	std::memcpy( mm_buf, ti, 2); mm_buf[2] = 0; ti += 2;
	char dd_buf[ 3];
	std::memcpy( dd_buf, ti, 2); dd_buf[2] = 0; ti += 2;
	if (*ti != '_') {errcode = ErrorCodeSyntax; return TimeStamp();}
	ti += 1;
	char HH_buf[ 3];
	std::memcpy( HH_buf, ti, 2); HH_buf[2] = 0; ti += 2;
	char MM_buf[ 3];
	std::memcpy( MM_buf, ti, 2); MM_buf[2] = 0; ti += 2;
	char SS_buf[ 3];
	std::memcpy( SS_buf, ti, 2); SS_buf[2] = 0; ti += 2;
	if (*ti != '_') {errcode = ErrorCodeSyntax; return TimeStamp();}
	ti += 1;
	char ii_buf[ 5];
	std::memcpy( ii_buf, ti, 4); ii_buf[4] = 0; ti += 4;

	if (!onlyDigits( YY_buf)
	||  !onlyDigits( mm_buf)
	||  !onlyDigits( dd_buf)
	||  !onlyDigits( HH_buf)
	||  !onlyDigits( MM_buf)
	||  !onlyDigits( SS_buf)
	||  !onlyDigits( ii_buf)) {errcode = ErrorCodeSyntax; return TimeStamp();}

	std::memset( &date, 0, sizeof(date));
	date.tm_sec  = ::atoi(SS_buf);
	date.tm_min  = ::atoi(MM_buf);
	date.tm_hour = ::atoi(HH_buf);
	date.tm_mday = ::atoi(dd_buf);
	date.tm_mon  = ::atoi(mm_buf)-1;
	date.tm_year = ::atoi(YY_buf) - 1900;
	date.tm_isdst = 0;

	int counter = ::atoi( ii_buf);
	time_t unixtime = ::mktime( &date);

	if (unixtime == ((time_t)-1))
	{
		errcode = ErrorCodeSyntax;
		return TimeStamp();
	}
	else
	{
		return TimeStamp( unixtime, counter);
	}
}

DLL_PUBLIC TimeStamp TimeStamp::fromdate( int year, int mon, int day, int hrs, int min, int sec, int cnt, ErrorCode& errcode)
{
	struct tm date;
	std::memset( &date, 0, sizeof(date));

	date.tm_sec  = sec;
	date.tm_min  = min;
	date.tm_hour = hrs;
	date.tm_mday = day;
	date.tm_mon  = mon-1;
	date.tm_year = year - 1900;
	date.tm_isdst = 0;

	time_t unixtime = ::mktime( &date);

	if (unixtime == ((time_t)-1))
	{
		errcode = ErrorCodeSyntax;
		return TimeStamp();
	}
	else
	{
		return TimeStamp( unixtime, cnt);
	}
}

static AtomicCounter<time_t> g_currentTime(0);
static AtomicCounter<int> g_currentTimeCounter(0);


static TimeStamp allocTimeStamp( ErrorCode& errcode)
{
AGAIN:
{
	// Get Value of current time and set the global variable with the current time if needed:
	time_t current_time = ::time(NULL);
	if (current_time == ((time_t)-1)) {errcode=ErrorCodeIOError; return TimeStamp();}

	int counter = g_currentTimeCounter.value();
	time_t ct = g_currentTime.value();
	if (current_time > ct)
	{
		if (!g_currentTime.test_and_set( ct, current_time)) goto AGAIN;
		if (!g_currentTimeCounter.test_and_set( counter, 0)) goto AGAIN;
		return TimeStamp( current_time, 0);
	}
	else
	{
		counter = g_currentTimeCounter.allocIncrement();
		return TimeStamp( ct, counter);
	}
}}

static TimeStamp currentTimeStamp( ErrorCode& errcode)
{
AGAIN:
{
	int counter = g_currentTimeCounter.value();
	time_t ct = g_currentTime.value();
	if (ct == 0)
	{
		time_t current_time = ::time(NULL);
		if (current_time == ((time_t)-1)) {errcode=ErrorCodeIOError; return TimeStamp();}
		if (!g_currentTime.test_and_set( ct, current_time)) goto AGAIN;
	}
	if (counter != g_currentTimeCounter.value()) goto AGAIN;
	return TimeStamp( ct, counter);
}}

DLL_PUBLIC TimeStamp TimeStamp::current( ErrorCode& errcode)
{
	return currentTimeStamp( errcode);
}

DLL_PUBLIC TimeStamp TimeStamp::alloc( ErrorCode& errcode)
{
	return allocTimeStamp( errcode);
}

DLL_PUBLIC std::string TimeStamp::tostring( const TimeStamp& timestamp, ErrorCode& errcode)
{
	try
	{
		if (!timestamp.defined()) return std::string();

		char timebuf[ 256];
		char timestampbuf[ 256];
	
		time_t tt = timestamp.unixtime();
		const struct tm* tm_info = ::localtime( &tt);
	
		std::strftime( timebuf, sizeof(timebuf), "%Y%m%d_%H%M%S", tm_info);
		std::snprintf( timestampbuf, sizeof(timestampbuf), "%s_%04d", timebuf, timestamp.counter() % 10000);
		return std::string(timestampbuf);
	}
	catch (const std::bad_alloc&)
	{
		errcode = ErrorCodeOutOfMem;
		return std::string();
	}
}

DLL_PUBLIC TimeStamp TimeStamp::fromstring( const std::string& timestampstr, ErrorCode& errcode)
{
	return parseTimeStamp( timestampstr.c_str(), errcode);
}

DLL_PUBLIC TimeStamp TimeStamp::fromstring( const char* timestampstr, ErrorCode& errcode)
{
	return parseTimeStamp( timestampstr, errcode);
}


