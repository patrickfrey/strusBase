/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of an algorithm approximating the size of the minimal cover of elements by sets
#include "strus/base/minimalCover.hpp"
#include "strus/base/bitset.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/errorCodes.hpp"
#include "private/internationalization.hpp"
#include "strus/errorBufferInterface.hpp"
#include <vector>
#include <iostream>
#include <set>
#include <algorithm>
#include <stdexcept>
#include <new>

#undef STRUS_LOWLEVEL_DEBUG

using namespace strus;

namespace {
struct ElementSetRelation
{
	int elemidx;
	std::size_t setidx;

	ElementSetRelation( int elemidx_, std::size_t setidx_)
		:elemidx(elemidx_),setidx(setidx_){}
	ElementSetRelation( const ElementSetRelation& o)
		:elemidx(o.elemidx),setidx(o.setidx){}

	bool operator < (const ElementSetRelation& o) const
	{
		return elemidx == o.elemidx ? setidx < o.setidx : elemidx < o.elemidx;
	}
};
}

DLL_PUBLIC MinimalCoverData::MinimalCoverData( const std::vector<std::vector<int> >& sets_, ErrorBufferInterface* errorhnd_)
	:m_sets(sets_),m_setlists(),m_invmap(),m_errorhnd(errorhnd_)
{
	try
	{
		std::set<ElementSetRelation> immap;
		std::vector<std::vector<int> >::const_iterator si = m_sets.begin(), se = m_sets.end();
		for (std::size_t sidx=0; si != se; ++si,++sidx)
		{
			std::vector<int>::const_iterator ei = si->begin(), ee = si->end();
			for (; ei != ee; ++ei)
			{
				immap.insert( ElementSetRelation( *ei, sidx));
			}
		}
		int prev_elemidx = -1;
		std::set<ElementSetRelation>::const_iterator mi = immap.begin(), me = immap.end();
		while (mi != me)
		{
			prev_elemidx = mi->elemidx;
			InvMapElement elem( 0/*size*/, m_setlists.size()/*list index*/);
			for (; mi != me && prev_elemidx == mi->elemidx; ++mi)
			{
				++elem.size;
				m_setlists.push_back( mi->setidx);
			}
			m_invmap.insert( InvMap::value_type( prev_elemidx, elem));
		}
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("memory allocation error"));
	}
	catch (const std::runtime_error& err)
	{
		m_errorhnd->report( ErrorCodeRuntimeError, _TXT("minimal cover approximation failed: %s"), err.what());
	}
}


void MinimalCoverData::minimalCoverSizeApproximationSubset( std::vector<int>& res, std::vector<int>::const_iterator& itr, const std::vector<int>::const_iterator& end, std::set<int>& elementsLeft) const
{
	// Build map of element indices to bit indices used in search:
	int elementCount = 0;
	typedef std::map<int,int> ElementMap;
	ElementMap elementMap;
	typedef bitset<SubsetSize> BitSet;
	std::set<BitSet> visited_sets;
	std::vector<BitSet> candidates;

	struct SetPick
	{
		int size;
		int candidateidx;
		std::size_t setidx;

		SetPick()
			:size(0),candidateidx(-1),setidx(0){}
		SetPick( int size_, int candidateidx_, std::size_t setidx_)
			:size(size_),candidateidx(candidateidx_),setidx(setidx_){}
		SetPick( const SetPick& o)
			:size(o.size),candidateidx(o.candidateidx),setidx(o.setidx){}

		bool valid() const
		{
			return size;
		}
	};
	SetPick candidatePicks[ SubsetSize];

	for (; itr != end && elementCount < SubsetSize; ++itr)
	{
		if (elementsLeft.find( *itr) != elementsLeft.end())
		{
			if (elementMap.insert( ElementMap::value_type( *itr, elementCount)).second == true/*insert took place*/)
			{
				++elementCount;
			}
		}
	}
	ElementMap::const_iterator ei = elementMap.begin(), ee = elementMap.end();
	for (; ei != ee; ++ei)
	{
		InvMap::const_iterator mi = m_invmap.find( ei->first);
		if (mi == m_invmap.end())
		{
			throw std::runtime_error( _TXT("not all elements covered by the sets defined in constructor"));
		}
		const InvMapElement& inv = mi->second;

		std::vector<std::size_t>::const_iterator si = m_setlists.begin() + inv.listindex;
		std::vector<std::size_t>::const_iterator se = si + inv.size;
		for (; si != se; ++si)
		{
			const std::vector<int>& st = m_sets[ *si];
			BitSet st_bits;
			std::vector<int>::const_iterator bi = st.begin(), be = st.end();
			for (; bi != be; ++bi)
			{
				ElementMap::const_iterator xi = elementMap.find( *bi);
				if (xi != elementMap.end())
				{
					st_bits.set( xi->second, true);
				}
			}
			if (visited_sets.insert( st_bits).second == true /*insert took place*/)
			{
				int st_size = st_bits.size();
				int fi = st_bits.first();
				for (; fi >= 0; fi = st_bits.next(fi))
				{
					SetPick& pick = candidatePicks[ fi];
					if (!pick.valid() || pick.size < st_size)
					{
						//... we have a new biggest set covering this element
						pick.size = st_size;
						pick.candidateidx = candidates.size();
						pick.setidx = *si;
					}
				}
				candidates.push_back( st_bits);
			}
		}
	}
	int fi = 0, fe = elementCount;
	for (; fi != fe; ++fi)
	{
		const SetPick& pick = candidatePicks[ fi];
		if (pick.candidateidx < 0)
		{
			throw std::runtime_error( _TXT("logic error: invalid candidate pick"));
		}
	}
	std::vector<int> rt;
	BitSet accu;
	while ((int)accu.size() < elementCount)
	{
		BitSet next;
		std::size_t setidx = 0;
		std::size_t maxsize = accu.size();
		bool changed = false;

		int pi=0, pe=elementCount;
		for (; pi != pe; ++pi)
		{
			const SetPick& pick = candidatePicks[ pi];
			BitSet cc = accu;
			cc.join( candidates[ pick.candidateidx]);
			if (cc.size() > maxsize)
			{
				maxsize = cc.size();
				next = cc;
				setidx = pick.setidx;
				changed = true;
			}
		}
		accu = next;
		if (!changed)
		{
			throw std::runtime_error( _TXT("logic error: invalid cover candiate sets leading to endless loop"));
		}
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "minimal cover grab {";
		std::vector<int>::const_iterator gi = m_sets[ setidx].begin(), ge = m_sets[ setidx].end();
		for (; gi != ge; ++gi)
		{
			if (elementsLeft.find( *gi) != elementsLeft.end()) std::cerr << " " << *gi;
		}
		std::cerr << " }" << std::endl;
#endif
		std::vector<int>::const_iterator ci = m_sets[ setidx].begin(), ce = m_sets[ setidx].end();
		for (; ci != ce; ++ci)
		{
			elementsLeft.erase( *ci);
		}
		res.push_back( setidx);
	}
}

DLL_PUBLIC std::vector<int> MinimalCoverData::minimalCoverApproximation( const std::vector<int>& elements) const
{
	std::vector<int> rt;
	try
	{
		std::set<int> elementsLeft( elements.begin(), elements.end());
		std::vector<int>::const_iterator ei = elements.begin(), ee = elements.end();
		while (ei != ee && !elementsLeft.empty())
		{
			minimalCoverSizeApproximationSubset( rt, ei, elements.end(), elementsLeft);
		}
		return rt;
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("memory allocation error"));
		rt.clear();
	}
	catch (const std::runtime_error& err)
	{
		m_errorhnd->report( ErrorCodeRuntimeError, _TXT("minimal cover approximation failed: %s"), err.what());
		rt.clear();
	}
	return rt;
}




