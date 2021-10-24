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
#include "strus/base/string_conv.hpp"
#include "strus/base/string_format.hpp"
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/time.h>
#include <cstdlib>

#define _TXT(str) str

using namespace strus;

static void adjustLeapSeconds( struct tm& date)
{
	if (date.tm_sec >= 60)
	{
		date.tm_min += 1;
		date.tm_sec -= 60;
	}
	if (date.tm_min >= 60)
	{
		date.tm_hour += 1;
		date.tm_min -= 60;
	}
}

static bool onlyDigits( char const* ti)
{
	for (;*ti >= '0' && *ti <= '9'; ++ti){}
	return !*ti;
}

static TimeStamp parseTimeStamp( const char* timestampstr)
{
	char const* ti = timestampstr;
	int timestamplen = 0;
	for (char const* li = ti; *li >= '0' && *li <= '9'; ++li,++timestamplen){}
	if (timestamplen != 14 && timestamplen != 17) return -1;
	struct tm date;

	char YY_buf[ 5];
	std::memcpy( YY_buf, ti, 4); YY_buf[4] = 0; ti += 4;
	char mm_buf[ 3];
	std::memcpy( mm_buf, ti, 2); mm_buf[2] = 0; ti += 2;
	char dd_buf[ 3];
	std::memcpy( dd_buf, ti, 2); dd_buf[2] = 0; ti += 2;
	char HH_buf[ 3];
	std::memcpy( HH_buf, ti, 2); HH_buf[2] = 0; ti += 2;
	char MM_buf[ 3];
	std::memcpy( MM_buf, ti, 2); MM_buf[2] = 0; ti += 2;
	char SS_buf[ 3];
	std::memcpy( SS_buf, ti, 2); SS_buf[2] = 0; ti += 2;
	char XX_buf[ 4];
	int millisec = 0;
	if (timestamplen >= 17)
	{
		std::memcpy( XX_buf, ti, 3); XX_buf[3] = 0; ti += 3;
		if (!onlyDigits( XX_buf)) return -1;
		millisec = ::atoi(XX_buf);
	}
	if (!onlyDigits( YY_buf)
	||  !onlyDigits( mm_buf)
	||  !onlyDigits( dd_buf)
	||  !onlyDigits( HH_buf)
	||  !onlyDigits( MM_buf)
	||  !onlyDigits( SS_buf)) {return -1;}

	std::memset( &date, 0, sizeof(date));
	date.tm_sec  = ::atoi(SS_buf);
	date.tm_min  = ::atoi(MM_buf);
	date.tm_hour = ::atoi(HH_buf);
	date.tm_mday = ::atoi(dd_buf);
	date.tm_mon  = ::atoi(mm_buf)-1;
	date.tm_year = ::atoi(YY_buf) - 1900;
	date.tm_isdst = 0;
	adjustLeapSeconds( date);

	time_t unixtime = ::mktime( &date);
	if (unixtime == ((time_t)-1)) return -1;
	unixtime -= timezone;
	return (int64_t)unixtime * 1000 + millisec;
}

DLL_PUBLIC TimeStamp strus::getCurrentTimeStamp()
{
	struct timeval tv;
	::gettimeofday( &tv, NULL);

	TimeStamp rt =
	    (int64_t)(tv.tv_sec + timezone) * 1000 +
	    (int64_t)(tv.tv_usec) / 1000;
	return rt;
}

DLL_PUBLIC TimeStampString strus::timeStampToString( const TimeStamp timestamp)
{
	TimeStampString rt;
	std::memset( rt.str, 0, sizeof(rt.str));
	if (timestamp < 0) return rt;

	char timebuf[ 32];

	time_t tt = timestamp / 1000;
	int millisec = timestamp % 1000;
	struct tm tmbuf;
	struct tm* tm_info = ::gmtime_r( &tt, &tmbuf);
	if (!tm_info) return rt;
	std::strftime( timebuf, sizeof(timebuf), "%Y%m%d%H%M%S", tm_info);
	std::snprintf( rt.str, sizeof(rt.str), "%s%03d", timebuf, millisec);
	return rt;
}

DLL_PUBLIC TimeStamp strus::timeStampFromString( const char* timestampstr)
{
	return parseTimeStamp( timestampstr);
}

DLL_PUBLIC TimeStamp strus::timeStampFromdate( int year, int mon, int day, int hrs, int min, int sec, int msec)
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
	adjustLeapSeconds( date);

	time_t unixtime = ::mktime( &date);
	if (unixtime == ((time_t)-1)) return -1;
	unixtime -= timezone;
	return (int64_t)unixtime * 1000 + msec;
}
