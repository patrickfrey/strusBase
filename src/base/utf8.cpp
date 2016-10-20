/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \file utf8.cpp
/// \brief Helpers for UTF-8 encoding/decoding
#include "strus/base/utf8.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/internationalization.hpp"

using namespace strus;

DLL_PUBLIC int32_t strus::utf8decode( const char* itr, unsigned int charsize)
{
	int32_t res = (unsigned char)*itr;
	if (res > 127)
	{
		res = ((unsigned char)*itr)&(B00011111>>(charsize-2));
		for (unsigned int ii=1; ii<charsize; ii++)
		{
			res <<= 6;
			res |= (unsigned char)(itr[ii] & B00111111);
		}
	}
	return res;
}

DLL_PUBLIC std::size_t strus::utf8encode( char* buf, int32_t chr)
{
	if (chr<0)
	{
		throw strus::runtime_error( _TXT( "illegal unicode character (%s)"), __FUNCTION__);
	}
	if (chr <= 127)
	{
		*buf = (char)(unsigned char)chr;
		return 1;
	}
	uint32_t pp,sf;
	for (pp=1,sf=5; pp<5; pp++,sf+=5)
	{
		if ((uint32_t)chr < (uint32_t)((1<<6)<<sf)) break;
	}
	unsigned char HB = (unsigned char)(B11111111 << (7-pp));
	unsigned char shf = (unsigned char)(pp*6);
	unsigned int ii;
	*buf++ = (char)(((unsigned char)(chr >> shf) & (~HB >> 1)) | HB);
	for (ii=1,shf-=6; ii<=pp; shf-=6,ii++)
	{
		*buf++ = (char)(unsigned char) (((chr >> shf) & B00111111) | B10000000);
	}
	return ii;
}



