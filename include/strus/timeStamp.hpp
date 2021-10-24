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
#include <ctime>
#include <string>
#include <cstdint>

namespace strus {

typedef int64_t TimeStamp;

struct TimeStampString
{
	char str[ 18];
};

/// \brief Get the current timestamp
/// \return the current time stamp (unix time in micro seconds)
TimeStamp getCurrentTimeStamp();

/// \brief Convert a timestamp into its readable string representation as YYYYMMDDhhmmssxxx
/// \return the string representation of the timestamp, empty in case of an invalid timestamp
TimeStampString timeStampToString( const TimeStamp timestamp);

/// \brief Parse a timestamp from its string representation
/// \return the timestamp represented by the parsed string or -1 in case of an invalid timestamp string
TimeStamp timeStampFromString( const char* timestampstr);

/// \brief Get a timestamp from its date and counter
/// \return the timestamp represented by the parameters or -1 in case of invalid arguments
TimeStamp timeStampFromdate( int year, int mon, int day, int hrs, int min, int sec, int msec);


}//namespace
#endif


