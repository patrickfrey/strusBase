/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Datatype used to mark time for ordering events and specifying snapshots
#ifndef _STRUS_STORAGE_TIMESTAMP_HPP_INCLUDED
#define _STRUS_STORAGE_TIMESTAMP_HPP_INCLUDED
#include "errorCodes.hpp"
#include <ctime>
#include <string>

namespace strus {

/// \brief Datatype used to mark time for ordering events and specifying snapshots
class TimeStamp
{
public:
	/// \brief Seconds since 1.1.1970
	time_t unixtime() const	{return m_unixtime;}
	/// \brief Counter for ordering events appearing in the same second
	int counter() const	{return m_counter;}

	/// \brief Constructor
	explicit TimeStamp( time_t unixtime_=0, int counter_=0)
		:m_unixtime(unixtime_),m_counter(counter_){}
	/// \brief Copy constructor
	TimeStamp( const TimeStamp& o)
		:m_unixtime(o.m_unixtime),m_counter(o.m_counter){}
	/// \brief Assignment
	TimeStamp& operator=( const TimeStamp& o)
		{m_unixtime=o.m_unixtime; m_counter=o.m_counter; return *this;}

	/// \brief Comparison
	bool operator == (const TimeStamp& o) const
		{return m_unixtime == o.m_unixtime && m_counter == o.m_counter;}
	bool operator != (const TimeStamp& o) const
		{return m_unixtime != o.m_unixtime || m_counter != o.m_counter;}
	bool operator >= (const TimeStamp& o) const
		{return m_unixtime == o.m_unixtime ? m_counter >= o.m_counter : m_unixtime >= o.m_unixtime;}
	bool operator > (const TimeStamp& o) const
		{return m_unixtime == o.m_unixtime ? m_counter > o.m_counter : m_unixtime > o.m_unixtime;}
	bool operator <= (const TimeStamp& o) const
		{return m_unixtime == o.m_unixtime ? m_counter <= o.m_counter : m_unixtime <= o.m_unixtime;}
	bool operator < (const TimeStamp& o) const
		{return m_unixtime == o.m_unixtime ? m_counter < o.m_counter : m_unixtime < o.m_unixtime;}

	// Implemented in libstrus_timestamp:

	/// \brief Get the current timestamp
	/// \param[out] errcode error code in case of error
	/// \return the current timestamp or TimeStamp() (with defined() == false) in case of error
	static TimeStamp current( ErrorCode& errcode);
	/// \brief Allocate a unique timestamp
	/// \param[out] errcode error code in case of error
	/// \return the current timestamp or TimeStamp() (with defined() == false) in case of error
	static TimeStamp alloc( ErrorCode& errcode);
	/// \brief Convert a timestamp into its string representation
	/// \param[out] errcode error code in case of error
	/// \return the string representation of the timestamp
	static std::string tostring( const TimeStamp& timestamp, ErrorCode& errcode);
	/// \brief Parse a timestamp from its string representation
	/// \param[out] errcode error code in case of error
	/// \return the timestamp represented by the parsed string or TimeStamp() (with defined() == false) in case of error
	static TimeStamp fromstring( const std::string& timestampstr, ErrorCode& errcode);
	/// \brief Parse a timestamp from its string representation
	/// \param[out] errcode error code in case of error
	/// \return the timestamp represented by the parsed string or TimeStamp() (with defined() == false) in case of error
	static TimeStamp fromstring( const char* timestampstr, ErrorCode& errcode);
	/// \brief Get a timestamp from its date and counter
	/// \param[out] errcode error code in case of error
	static TimeStamp fromdate( int year, int mon, int day, int hrs, int min, int sec, int cnt, ErrorCode& errcode);

	bool defined() const
	{
		return !!m_unixtime;
	}

private:
	time_t m_unixtime;		//< seconds since 1.1.1970
	int m_counter;			//< counter for ordering events appearing in the same second
};

}//namespace
#endif


