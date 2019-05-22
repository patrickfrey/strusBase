/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Base64 encoding of binary blobs
#ifndef _STRUS_BASE_BASE64_HPP_INCLUDED
#define _STRUS_BASE_BASE64_HPP_INCLUDED
#include "strus/errorCodes.hpp"
#include <utility>

namespace strus
{

/// \brief Get the needed length of a buffer for encoding a binary blob to base64
/// \param[in] srclen length of the binary blob
/// \return the needed length of the destination buffer in bytes
std::size_t base64EncodeLength( std::size_t srclen);

/// \brief Get the needed length of a buffer for decoding a binary blob from base64
/// \param[in] bloblen length of the binary blob
/// \return the needed length of the destination buffer in bytes
std::size_t base64DecodeLength( const char* encoded, std::size_t encodedlen);

/// \brief Encode a binary blob as base64
/// \param[out] destbuf buffer where to write the encoded blob to
/// \param[in] destbufsize allocation size of the destination buffer in bytes
/// \param[in] blob binary blob to encode in base64
/// \param[in] bloblen length of the binary blob
/// \param[out] errcode error code in case of error
/// \return the length of the result in bytes or 0 in case of an error, see errcode for the reason of failure
std::size_t encodeBase64( char* destbuf, std::size_t destbufsize, const void* blob, std::size_t bloblen, ErrorCode& errcode);

/// \brief Decode a binary blob from base64
/// \param[out] blobbuf buffer where to write the decoded data to
/// \param[in] blobbufsize allocation size of the destination buffer in bytes
/// \param[in] encoded base64 encoded string to decode from base64
/// \param[in] encodedlen length of the encoded string
/// \param[out] errcode error code in case of error
/// \return the length of the result in bytes or 0 in case of an error, see errcode for the reason of failure
std::size_t decodeBase64( void* blobbuf, std::size_t blobbufsize, const char* encoded, std::size_t encodedlen, ErrorCode& errcode);

}//namespace
#endif


