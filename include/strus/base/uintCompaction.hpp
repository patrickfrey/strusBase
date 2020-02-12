/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_BASE_UINT_COMPACTION_HPP_INCLUDED
#define _STRUS_BASE_UINT_COMPACTION_HPP_INCLUDED
/// \brief Functions to represent an approximation value of a 16 bit unsigned int value as a byte with some loss
/// \note Used in the storage to represent feature frenquencies compacted to a byte
#include "strus/base/stdint.h"

namespace strus
{

/// \brief Map a 16 bit unsigned integer value to a byte with some information loss
/// \param[in] in unsigned integer value to compress
/// \return compacted value
uint8_t compactUint( unsigned int in);

/// \brief Map a compacted value representation back to the upper bound of all values represented by the input byte
/// \param[in] value byte representing a compacted unsigned integer value
/// \return upper bound of all values represented by the parameter
unsigned int uintFromCompaction( uint8_t in);

}//namespace
#endif

