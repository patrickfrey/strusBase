/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \file utf8.hpp
/// \brief Helpers for UTF-8 encoding/decoding
#ifndef _STRUS_BASE_UTF8_ENCODING_DECODING_HPP_INCLUDED
#define _STRUS_BASE_UTF8_ENCODING_DECODING_HPP_INCLUDED
#include "strus/base/bitOperations.hpp"
#include "strus/base/stdint.h"

namespace strus
{
enum {
	B11111111=0xFF,
	B01111111=0x7F,
	B00111111=0x3F,
	B00011111=0x1F,
	B00001111=0x0F,
	B00000111=0x07,
	B00000011=0x03,
	B00000001=0x01,
	B00000000=0x00,
	B10000000=0x80,
	B11000000=0xC0,
	B11100000=0xE0,
	B11110000=0xF0,
	B11111000=0xF8,
	B11111100=0xFC,
	B11111110=0xFE,

	B11011111=B11000000|B00011111,
	B11101111=B11100000|B00001111,
	B11110111=B11110000|B00000111,
	B11111011=B11111000|B00000011,
	B11111101=B11111100|B00000001
};

/// \brief Return true, if the character passed as argument is a non start character of a multi byte encoded unicode character
static inline bool utf8midchr( unsigned char ch)
{
	return (ch & B11000000) == B10000000;
}

/// \brief Skip to the begin of an UTF-8 encoded character from a pointer into it
/// \param[in] pointer into the UTF-8 encoded character
/// \return pointer to the start of the character
static inline const char* utf8prev( char const* end)
{
	for (--end; ((unsigned char)*end & B11000000) == B10000000; --end){}
	return end;
}

/// \brief Get the lenght of an UTF-8 encoded character from its first byte
/// \param[in] the first byte of the character
/// \return the length of the character in bytes
static inline unsigned char utf8charlen( unsigned char ch)
{
	unsigned char cl = 9-BitOperations::bitScanReverse( (uint8_t)(ch^0xFF));
	return cl>2?(cl-1):1;
}

/// \brief Decoding of a single UTF-8 character in a string
int32_t utf8decode( const char* itr, unsigned int charsize);

/// \brief Encoding of a single UTF-8 character into a string buffer
std::size_t utf8encode( char* buf, int32_t chr);

} //namespace
#endif


