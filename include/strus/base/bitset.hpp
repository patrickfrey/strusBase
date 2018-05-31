/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Bitset implementation
/// \note We do not use STL <bitset> because we need insert and remove and an iterator, besides toggling bits
#ifndef _STRUS_BITSET_HPP_INCLUDED
#define _STRUS_BITSET_HPP_INCLUDED
#include "strus/base/bitOperations.hpp"
#include "strus/base/stdint.h"
#include <vector>

namespace strus {

template <int SIZE>
struct bitset_hi_bitmask
{
	static uint64_t mask( uint64_t hi)	{return hi & (((uint64_t)1 << (SIZE % 64)) -1);}
};
template <>
struct bitset_hi_bitmask<0>
{
	static uint64_t mask( uint64_t hi)	{return hi;}
};


template <int SIZE>
class bitset
{
public:
	bitset()
	{
		int ii=0;
		for (; ii<ArSize; ++ii)
		{
			m_ar[ ii] = 0;
		}
	}
	bitset( const bitset& o)
	{
		int ii=0;
		for (; ii<ArSize; ++ii)
		{
			m_ar[ ii] = o.m_ar[ ii];
		}
	}

	bool set( int pos, bool value)
	{
		if ((unsigned int)pos >= SIZE) return false;
		int idx = (unsigned int)pos / 64;
		int ofs = (unsigned int)pos % 64;
		if (value)
		{
			m_ar[ idx] |= ((uint64_t)1 << ofs);
		}
		else
		{
			m_ar[ idx] &= ~((uint64_t)1 << ofs);
		}
		return true;
	}

	bool test( int pos) const
	{
		if ((unsigned int)pos >= SIZE) return false;
		int idx = (unsigned int)pos / 64;
		int ofs = (unsigned int)pos % 64;
		return (m_ar[ idx] & ((uint64_t)1 << ofs)) != 0;
	}

	bool insert( int pos, bool value)
	{
		if ((unsigned int)pos >= SIZE) return false;
		unsigned int idx = (unsigned int)pos / 64;
		unsigned int ofs = (unsigned int)pos % 64;
		unsigned int ii = ArSize-1;
		for (; ii > idx; --ii)
		{
			if (ii < ArSize-1)
			{
				m_ar[ ii+1] |= (m_ar[ ii] & ((uint64_t)1UL << 63)) >> 63;
			}
			m_ar[ ii] <<= 1;
		}
		if (ii < ArSize-1)
		{
			m_ar[ ii+1] |= (m_ar[ ii] & ((uint64_t)1UL << 63)) >> 63;
		}
		uint64_t left( m_ar[ idx]);
		uint64_t right( left);
		uint64_t newbit = ((uint64_t)value << ofs);
		left >>= ofs;
		left <<= ofs;
		right ^= left;
		left <<= 1;
		left |= newbit;
		m_ar[ idx] = left | right;
		if (idx == ArSize-1)
		{
			m_ar[ idx] = bitset_hi_bitmask<SIZE % 64>::mask( m_ar[ idx]);
		}
		return true;
	}

	bool remove( int pos)
	{
		if ((unsigned int)pos >= SIZE) return false;
		unsigned int idx = (unsigned int)pos / 64;
		unsigned int ofs = (unsigned int)pos % 64;
		uint64_t hibit = 0;
		unsigned int ii = ArSize-1;
		for (; ii > idx; --ii)
		{
			uint64_t lobit = m_ar[ ii] & 1;
			m_ar[ ii] >>= 1;
			m_ar[ ii] |= hibit;
			hibit = lobit << 63;
		}
		if (ofs == 63)
		{
			//... this case has to be handled specially, because behavior of >> 64 undefined
			m_ar[ idx] &= ~((uint64_t)1 << 63);
			m_ar[ idx] |= hibit;
		}
		else
		{
			uint64_t left( m_ar[ idx]);
			uint64_t delbit = left & ((uint64_t)1 << ofs);
			uint64_t right( left ^ delbit);
			left >>= (ofs+1);
			left <<= (ofs+1);
			right ^= left;
			left >>= 1;
			left |= hibit;
			m_ar[ ii] = left | right;
		}
		return true;
	}

	void reset()
	{
		int ai = 0;
		for (; ai < ArSize; ++ai) m_ar[ai] = 0;
	}

	int next( int pos) const
	{
		++pos;
		int idx = (unsigned int)pos / 64;
		int ofs = (unsigned int)pos % 64;
		if (idx >= ArSize) return -1;
		uint64_t start = m_ar[ idx];
		start &= ~((((uint64_t)1) << ofs) - 1);
		if (start)
		{
			int pi = BitOperations::bitScanForward( start);
			return idx * 64 + pi - 1;
		}
		for (++idx; idx < ArSize; ++idx)
		{
			if (m_ar[ idx])
			{
				int pi = BitOperations::bitScanForward( m_ar[ idx]);
				return idx * 64 + pi - 1;
			}
		}
		return -1;
	}
	int first() const
	{
		return next( -1);
	}

	std::vector<int> elements() const
	{
		std::vector<int> rt;
		int pi = first();
		for (; pi >= 0; pi = next(pi))
		{
			rt.push_back( pi);
		}
		return rt;
	}

	bool empty() const
	{
		for (int ai=0; ai<ArSize; ++ai)
		{
			if (m_ar[ai]) return false;
		}
		return true;
	}

private:
	enum {ArSize=(SIZE+63)/64};
	uint64_t m_ar[ ArSize];
};

}//namespace
#endif


