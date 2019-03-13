/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Wrapper to dynamic bitset implementation
#ifndef _STRUS_DYNAMIC_BITSET_HPP_INCLUDED
#define _STRUS_DYNAMIC_BITSET_HPP_INCLUDED
#include "strus/base/bitset.hpp"
#include <utility>

/// PF:HACK: Bad solution, need probing of dynamic_bitset as C++ feature as for regex
#if defined __GNUC__
#define STRUS_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#undef STRUS_USE_STD_DYNAMIC_BITSET
#if __cplusplus >= 201103L
#if defined __clang__
#define STRUS_USE_STD_DYNAMIC_BITSET
#elif defined __GNUC__
#if STRUS_GCC_VERSION >= 40900
#define STRUS_USE_STD_DYNAMIC_BITSET
#endif // STRUS_GCC_VERSION
#endif // __clang__
#endif // __cplusplus

#ifdef STRUS_USE_STD_DYNAMIC_BITSET
#include <vector>
#include <algorithm>
#include "strus/base/stdint.h"
#include <limits>
namespace strus {

/// \brief Bitset with a dimension defined by the constructor
/// \remark Possible alternative: Library RoaringBitmap (see https://github.com/RoaringBitmap/CRoaring/blob/master/include/roaring/roaring.h)
class dynamic_bitset
{
public:
	explicit dynamic_bitset( std::size_t size_)
		:m_indices( (size_+(ElementDim-1)) / ElementDim, -1),m_elements()
	{
		if (size_ > (std::size_t)std::numeric_limits<int32_t>::max()) throw std::bad_alloc();
	}
	dynamic_bitset( const dynamic_bitset& o)
		:m_indices(o.m_indices),m_elements(o.m_elements){}
	dynamic_bitset& operator=( const dynamic_bitset& o)
		{m_indices = o.m_indices; m_elements = o.m_elements; return *this;}
#if __cplusplus >= 201103L
	dynamic_bitset( dynamic_bitset&& o)
		:m_indices(std::move(o.m_indices)),m_elements(std::move(o.m_elements)){}
	dynamic_bitset( dynamic_bitset&& o)
		{m_indices=std::move(o.m_indices); m_elements=std::move(o.m_elements); return *this;}
#endif
	bool set( std::size_t n, bool val = true)
	{
		int hi = n / ElementDim;
		int li = n % ElementDim;
		if (hi >= (int)m_indices.size()) return false;
		int idx = m_indices[ hi];
		if (idx == -1)
		{
			idx = m_indices[ hi] = m_elements.size();
			m_elements.push_back( bitset<ElementDim>());
		}
		return m_elements[ idx].set( li, val);
	}
	bool test( std::size_t n) const
	{
		int hi = n / ElementDim;
		int li = n % ElementDim;
		if (hi >= (int)m_indices.size()) return false;
		int idx = m_indices[ hi];
		if (idx == -1) return false;
		return m_elements.at(idx).test( li);
	}
	void reset()
	{
		m_elements.clear();
		std::fill( m_indices.begin(), m_indices.end(), -1);
	}

private:
	enum {ElementDim=256};
	std::vector<int32_t> m_indices;
	std::vector<bitset<ElementDim> > m_elements;
};
}//namespace

#else //STRUS_USE_STD_DYNAMIC_BITSET
#include <boost/dynamic_bitset.hpp>

namespace strus {

class dynamic_bitset
	:public boost::dynamic_bitset<>
{
public:
	dynamic_bitset( std::size_t size_)
		:boost::dynamic_bitset<>( size_){}
	dynamic_bitset( const dynamic_bitset& o)
		:boost::dynamic_bitset<>( o){}
};

}//namespace
#endif //STRUS_USE_STD_DYNAMIC_BITSET

#endif


