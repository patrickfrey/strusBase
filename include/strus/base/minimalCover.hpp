/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of an algorithm approximating the size of the minimal cover of elements by sets
/// \note Because of the NP-hardness of the problem, we are calculating an approximation and do not guarantee to provide a solution. 
/// \nore See https://en.wikipedia.org/wiki/Set_cover_problem
#ifndef _STRUS_MINIMAL_COVER_HPP_INCLUDED
#define _STRUS_MINIMAL_COVER_HPP_INCLUDED
#include <vector>
#include <map>
#include <set>

namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Implementation of an algorithm approximating the size of the minimal cover of elements by sets
class MinimalCoverData
{
public:
	/// \brief Constructor initializing data used for calculation
	/// \param[in] sets covering sets of elements
	MinimalCoverData( const std::vector<std::vector<int> >& sets_, ErrorBufferInterface* errorhnd_);

	/// \brief Make an approximation the minimal set cover
	/// \param[in] elements elements to cover by the sets defined in the constructor
	/// \return a minimal cover approximation candidate (vector of set indices starting with 0) in case of success, an empty set in case of a failure
	std::vector<int> minimalCoverApproximation( const std::vector<int>& elements) const;

private:
	/// \brief Size of subset used for partial approximation (64 is used because the set representation fits into a int64_t)
	enum {SubsetSize=64};

	/// \brief Approximation that solves the problem for subsets. The caller sums up the results to get the total approximation.
	/// \paran[out] where to append result to
	/// \paran[in,out] itr in iterator to start with, out iterator for next call
	/// \paran[in] end end of input
	/// \param[in,out] elementsLeft set of elements not yet covered by previous calls. Inittialized by the caller
	void minimalCoverSizeApproximationSubset( std::vector<int>& res, std::vector<int>::const_iterator& itr, const std::vector<int>::const_iterator& end, std::set<int>& elementsLeft) const;

private:
	std::vector<std::vector<int> > m_sets;
	std::vector<std::size_t> m_setlists;
	struct InvMapElement
	{
		std::size_t size;
		std::size_t listindex;

		InvMapElement()
			:size(0),listindex(0){}
		InvMapElement( std::size_t size_, std::size_t listindex_)
			:size(size_),listindex(listindex_){}
		InvMapElement( const InvMapElement& o)
			:size(o.size),listindex(o.listindex){}
		InvMapElement& operator = ( const InvMapElement& o)
			{size=o.size; listindex=o.listindex; return *this;}
	};
	typedef std::map<int,InvMapElement> InvMap;
	InvMap m_invmap;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif


