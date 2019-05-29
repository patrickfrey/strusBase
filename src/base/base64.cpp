/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Base64 encoding of binary blobs
#include "strus/base/base64.hpp"
#include "strus/base/dll_tags.hpp"
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>
using namespace strus;

DLL_PUBLIC std::size_t strus::base64EncodeLength( std::size_t srclen)
{
	std::size_t blocks = (srclen + 2) / 3;
	std::size_t bytes = blocks * 4;
	return bytes;
}

DLL_PUBLIC std::size_t strus::base64DecodeLength( const char* encoded, std::size_t encodedlen)
{
	if (encodedlen < 4) return 0;
	std::size_t padding = encoded[encodedlen-1] == '='
			? (encoded[encodedlen-2] == '=' ? 2 : 1)
			: 0;
	std::size_t blocks = encodedlen / 4;
	std::size_t bytes = blocks * 3 - padding;
	return bytes;
}

DLL_PUBLIC std::size_t strus::encodeBase64( char* destbuf, std::size_t destbufsize, const void* blob, std::size_t bloblen, ErrorCode& errcode)
{
	namespace bai=boost::archive::iterators;
	typedef bai::base64_from_binary< bai::transform_width< const char *, 6, 8> > base64_text;
	base64_text bi = base64_text( (const char*)blob);
	base64_text be = base64_text( (const char*)blob + bloblen);
	std::size_t di = 0, de = destbufsize;
	for (; di != de && bi != be; ++di,++bi)
	{
		destbuf[di] = *bi;
	}
	if (bi != be)
	{
		errcode = ErrorCodeBufferOverflow;
		return 0;
	}
	switch (bloblen % 3)
	{
		case 0: break;
		case 1: if (di == de) {errcode = ErrorCodeBufferOverflow; return 0;} else destbuf[di++] = '='; /*no break here!*/
		case 2: if (di == de) {errcode = ErrorCodeBufferOverflow; return 0;} else destbuf[di++] = '='; /*no break here!*/
	}
	return di;
}

DLL_PUBLIC std::size_t strus::decodeBase64( void* blobbuf, std::size_t blobbufsize, const char* encoded, std::size_t encodedlen, ErrorCode& errcode)
{
	namespace bai=boost::archive::iterators;
	typedef bai::transform_width<bai::binary_from_base64<const char *>, 8, 6> base64_dec;

	std::size_t bi = 0, be = base64DecodeLength( encoded, encodedlen);
	base64_dec ci = base64_dec( encoded);
	base64_dec ce = base64_dec( encoded + encodedlen);
	if (blobbufsize < be)
	{
		errcode = ErrorCodeBufferOverflow;
		return 0;
	}
	for (; ci != ce && bi != be; ++ci,++bi)
	{
		((char*)blobbuf)[ bi] = *ci;
	}
	if (ci != ce)
	{
		errcode = ErrorCodeLogicError;
		return 0;
	}
	if (bi != be)
	{
		errcode = ErrorCodeSyntax;
		return 0;
	}
	return bi;
}



