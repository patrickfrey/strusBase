/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Method to calculate CRC32 values that can be used for hashing or checksums in messages
#ifndef _STRUS_BASE_CRC32_HPP_INCLUDED
#define _STRUS_BASE_CRC32_HPP_INCLUDED
#include <utility>
#include <cstddef> 
#include "strus/base/stdint.h"

namespace strus {
namespace utils {

/// \brief Class with some functions to calculate a CRC32 value of a buffer (memory area)
class Crc32
{
public:
	static uint32_t calc( const char* blk, std::size_t blksize);
	static uint32_t calc( const char* blk);
};

}}
#endif

