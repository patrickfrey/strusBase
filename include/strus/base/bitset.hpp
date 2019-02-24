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
	/// \brief Constructor
	bitset()
	{
		int ii=0;
		for (; ii<ArSize; ++ii)
		{
			m_ar[ ii] = 0;
		}
	}
	/// \brief Copy constructor
	bitset( const bitset& o)
	{
		int ii=0;
		for (; ii<ArSize; ++ii)
		{
			m_ar[ ii] = o.m_ar[ ii];
		}
	}
	/// \brief Assignment operator
	bitset& operator = ( const bitset& o)
	{
		int ii=0;
		for (; ii<ArSize; ++ii)
		{
			m_ar[ ii] = o.m_ar[ ii];
		}
		return *this;
	}

	/// \brief Toggle a bit on a defined position
	/// \return true, if the content of the set has changed with the operation
	bool set( int pos, bool value)
	{
		if ((unsigned int)pos >= SIZE) return false;
		int idx = (unsigned int)pos / 64;
		int ofs = (unsigned int)pos % 64;
		uint64_t& newval = m_ar[ idx];
		uint64_t oldval = newval;
		if (value)
		{
			newval |= ((uint64_t)1 << ofs);
		}
		else
		{
			newval &= ~((uint64_t)1 << ofs);
		}
		return newval != oldval;
	}

	/// \brief Test a bit on a defined position
	/// \return the value of the bit as boolean
	bool test( int pos) const
	{
		if ((unsigned int)pos >= SIZE) return false;
		int idx = (unsigned int)pos / 64;
		int ofs = (unsigned int)pos % 64;
		return (m_ar[ idx] & ((uint64_t)1 << ofs)) != 0;
	}

	/// \brief Insert a bit on a defined position with a shift of the right bits, the rightmost bit of the array is lost with this operation
	/// \return true if the insert took place, false on an array bound write rejected
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

	/// \brief Remove a bit on a defined position with a left shift of the right bits, the end bit of the array is zeroed
	/// \return true if the remove took place, false on an array bound read/write rejected
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

	/// \brief Zero all bits of the set
	void reset()
	{
		int ai = 0;
		for (; ai < ArSize; ++ai) m_ar[ai] = 0;
	}

	/// \brief Get the next right/upper bit of the set with position strictly higher than the position passed as argument
	/// \note useful for iterating through the bits set
	/// \return the position of the bit starting from 0'
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
	/// \brief Get the first set bit of the set
	/// \return the position of the bit starting from 0'
	int first() const
	{
		return next( -1);
	}

	/// \brief Get the indices of the set bits in the set
	/// \return the array of positions of the bit starting from 0
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

	/// \brief Test if the set is empty (contains no bits set)
	/// \return true if yes, false if no
	bool empty() const
	{
		for (int ai=0; ai<ArSize; ++ai)
		{
			if (m_ar[ai]) return false;
		}
		return true;
	}

	/// \brief Add the elements of an equally dimensioned set
	/// \return true if operation changed the contents of the set
	bool join( const bitset<SIZE>& o)
	{
		bool rt = false;
		for (int ai=0; ai<ArSize; ++ai)
		{
			uint64_t ee = m_ar[ai] | o.m_ar[ai];
			rt |= (ee != m_ar[ai]);
			m_ar[ai] = ee;
		}
		return rt;
	}

	/// \brief Get the number of set bits in the set
	/// \return the number of non-zero bits
	std::size_t size() const
	{
		std::size_t rt = 0;
		for (int ai=0; ai<ArSize; ++ai)
		{
			rt += BitOperations::bitCount( m_ar[ai]);
		}
		return rt;
	}

	/// \brief Comparison (lesser) of two sets
	/// \return true if yes, false if no
	bool operator < (const bitset& o) const
	{
		int ai = 0;
		for (; ai<ArSize && m_ar[ai] == o.m_ar[ai]; ++ai){}
		return ai == ArSize ? false : m_ar[ai] < o.m_ar[ai];
	}

	/// \brief Test if two sets are equal
	/// \return true if yes, false if no
	bool operator == (const bitset& o) const
	{
		int ai = 0;
		for (; ai<ArSize && m_ar[ai] == o.m_ar[ai]; ++ai){}
		return ai == ArSize;
	}

private:
	enum {ArSize=(SIZE+63)/64};
	uint64_t m_ar[ ArSize];
};

}//namespace
#endif


