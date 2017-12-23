/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Wrapper to hash table implementation
#ifndef _STRUS_UNORDERED_MAP_HPP_INCLUDED
#define _STRUS_UNORDERED_MAP_HPP_INCLUDED

/// PF:HACK: Bad solution, need probing of unordered_map as C++ feature as for regex
#if defined __GNUC__
#define STRUS_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#undef STRUS_USE_STD_UNORDERED_MAP
#if __cplusplus >= 201103L
#if defined __clang__
#define STRUS_USE_STD_UNORDERED_MAP
#elif defined __GNUC__
#if STRUS_GCC_VERSION >= 40900
#define STRUS_USE_STD_UNORDERED_MAP
#endif // STRUS_GCC_VERSION
#endif // __clang__
#endif // __cplusplus

#ifdef STRUS_USE_STD_UNORDERED_MAP
#include <unordered_map>
#include <functional>

namespace strus {

template<typename Key, typename Elem, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key> >
class unordered_map
	:public std::unordered_map<Key,Elem,Hash,Pred>
{
public:
	unordered_map(){}
	unordered_map( const unordered_map& o)
		:std::unordered_map<Key,Elem>(){}
};
}//namespace

#else //STRUS_USE_STD_UNORDERED_MAP
#include <boost/unordered_map.hpp>
#include <functional>

namespace strus {

template<typename Key, typename Elem, typename Hash = boost::hash<Key>, typename Pred = std::equal_to<Key> >
class unordered_map
	:public boost::unordered_map<Key,Elem,Hash,Pred>
{
public:
	unordered_map(){}
	unordered_map( const unordered_map& o)
		:boost::unordered_map<Key,Elem>(){}
};
}//namespace
#endif //STRUS_USE_STD_UNORDERED_MAP

#endif

