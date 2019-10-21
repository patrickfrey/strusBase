/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Set of ASCII characters defined as bit set
#ifndef _STRUS_ASCII_CHAR_SET_HPP_INCLUDED
#define _STRUS_ASCII_CHAR_SET_HPP_INCLUDED
#include "strus/base/stdint.h"
#include <cstring>

namespace strus {

struct AsciiCharSet
{
public:
	AsciiCharSet( const char* chrs)
	{
		std::memset( m_ar, 0, sizeof(m_ar));
		char const* ci = chrs;
		while (*ci) set( (unsigned char)*ci++);
	}

	bool test( char ch) const
	{
		return (m_ar[ (ch >> 6) & 0xFF] & ((uint64_t)1 << (ch & 63))) != 0;
	}

private:
	void set( unsigned char ch)
	{
		m_ar[ (ch >> 6) & 0xFF] |= (uint64_t)1 << (ch & 63);
	}

private:
	uint64_t m_ar[ 4];
};

}//namespace
#endif

