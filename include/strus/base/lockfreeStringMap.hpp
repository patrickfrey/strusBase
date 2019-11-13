/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of a hash table for shared read/write trying to avoid locking for parallel read accesses and single write access
/// \file lockfreeStringMap.hpp
#ifndef _STRUS_BASE_LOCKFREE_STRING_MAP_HPP_INCLUDED
#define _STRUS_BASE_LOCKFREE_STRING_MAP_HPP_INCLUDED
#include "strus/base/atomic.hpp"
#include "strus/base/thread.hpp"
#include "strus/base/shared_ptr.hpp"
#include "strus/base/crc32.hpp"
#include "strus/base/stdint.h"
#include <limits>
#include <map>
#include <string>
#include <cstring>

#undef STRUS_USE_STD_MAKE_SHARED
#if __cplusplus >= 201103L
#if defined __clang__
#define STRUS_USE_STD_MAKE_SHARED
#elif defined __GNUC__
#if STRUS_GCC_VERSION >= 40900
#define STRUS_USE_STD_MAKE_SHARED
#endif // STRUS_GCC_VERSION
#endif // __clang__
#endif // __cplusplus

#ifdef STRUS_USE_STD_MAKE_SHARED
#include <memory>
#else
#include <boost/make_shared.hpp> 
#endif


namespace strus
{

/// \brief Lockfree map of string to a scalar or atomically assignable value
/// \note ValueType must be atomically assignable; there is just one writer at once but values are assigned by the assignment operator
/// \note For atomic data types see https://www.gnu.org/software/libc/manual/html_node/Atomic-Types.html
template <typename ValueType>
class LockfreeStringMap
{
public:
	explicit LockfreeStringMap( std::size_t nofBlocks=128)
		:m_hashMask(1),m_nofHashBits(0),m_blocks()
	{
		if (nofBlocks <= 1) nofBlocks = 1;
		if (nofBlocks >= (std::size_t)(1<<31)) throw std::bad_alloc();
		while (m_hashMask < (uint32_t)nofBlocks) {++m_nofHashBits; m_hashMask <<= 1;}
		for (std::size_t ii=0; ii<m_hashMask; ++ii) m_blocks.push_back( BlockKeyMapRef());
		m_hashMask--;
	}

	bool get( const char* key, ValueType& value) const
	{
		std::size_t blockidx = hashString( key) & m_hashMask;

		BlockKeyMapRef ownership = m_blocks[ blockidx];
		if (!ownership.get()) return false;
		const ValueType* valref = ownership->find( key);
		if (valref)
		{
			value = *valref;
			return true;
		}
		else
		{
			return false;
		}
	}

	class Assignment
	{
	public:
		Assignment(){}
		void operator()( int& dest, int source) const
		{
			dest = source;
		}
	};

	template <class Updater>
	void set( const char* key, const ValueType& value, const Updater& updater = Updater())
	{
		std::size_t blockidx = hashString( key) & m_hashMask;

		strus::unique_lock lock( m_writerMutex); //... only one writer
		if (!m_blocks[ blockidx].get())
		{
			m_blocks[ blockidx] = BlockKeyMap::newBlock( key, value);
		}
		else
		{
			BlockKeyMapRef newblock = m_blocks[ blockidx]->setValueAndGetUpdatedBlock( key, value, updater);
			if (newblock.get()) m_blocks[ blockidx] = newblock;
		}
	}

	template <class Updater, class KeyValuePairList>
	void set( const KeyValuePairList& assignmentList, Updater updater = Assignment())
	{
		typedef typename KeyValuePairList::value_type KeyValuePair;
		typedef std::vector<KeyValuePair> KeyValuePairVector;
		typedef std::map<std::size_t,KeyValuePairVector> BlockMap;

		BlockMap blockmap;

		typename KeyValuePairList::const_iterator ai = assignmentList.begin(), ae = assignmentList.end();
		for (; ai != ae; ++ai)
		{
			std::size_t blockidx = hashString( ai->first) & m_hashMask;
			blockmap[ blockidx].push_back( *ai);
		}

		typename BlockMap::const_iterator bi = blockmap.begin(), be = blockmap.end();
		for (; bi != be; ++bi)
		{
			strus::unique_lock lock( m_writerMutex); //... only one writer

			BlockKeyMapRef newblock = BlockKeyMap::mergeBlock( m_blocks[ bi->first].get(), bi->second, updater);
			if (newblock.get()) m_blocks[ bi->first] = newblock;
		}
	}

private:
	struct BlockKeyMap;
#ifdef STRUS_USE_STD_MAKE_SHARED
	typedef std::shared_ptr<BlockKeyMap> BlockKeyMapRef;
	static BlockKeyMapRef createBlockKeyMapRef() {return std::make_shared<BlockKeyMap>();}
#else
	typedef boost::shared_ptr<BlockKeyMap> BlockKeyMapRef;
	static BlockKeyMapRef createBlockKeyMapRef() {return boost::make_shared<BlockKeyMap>();}
#endif

	struct BlockKeyMap
	{
		struct KeyLess
		{
			bool operator()( char const *a, char const *b) const
			{
				return std::strcmp( a, b) < 0;
			}
		};

		typedef std::map<const char*,ValueType,KeyLess> Map;
		Map map;
		std::string strar;

		BlockKeyMap() :map(),strar(){}

		static BlockKeyMapRef newBlock( const char* key, ValueType value)
		{
			BlockKeyMapRef copy = createBlockKeyMapRef();
			copy->strar = key;
			copy->map.insert( typename Map::value_type( copy->strar.c_str(), value));
			return copy;
		}

		template <class Updater, class KeyValuePairList>
		static BlockKeyMapRef mergeBlock( BlockKeyMap* ths, const KeyValuePairList& assignmentList, Updater updater)
		{
			BlockKeyMapRef copy = createBlockKeyMapRef();
			typename KeyValuePairList::const_iterator ai = assignmentList.begin(), ae = assignmentList.end();
			std::vector<ValueType> values;
			for (; ai != ae; ++ai)
			{
				typename Map::iterator mi;
				std::string key = ai->first;
				if (ths && ths->map.end() != (mi = ths->map.find( key.c_str())))
				{
					// ... key already defined, update value in table
					updater( mi->second, ai->second);
				}
				else
				{
					// ... key unknown, create new key and add value to parallel array values
					values.push_back( ai->second);
					if (!copy->strar.empty()) copy->strar.push_back( '\0');
					copy->strar.append( key);
				}
			}
			if (values.empty())
			{
				return BlockKeyMapRef();
			}
			if (ths)
			{
				typename Map::const_iterator oi = ths->map.begin(), oe = ths->map.end();
				for (; oi != oe; ++oi)
				{
					// ... add all old keys to new keys and add value to parallel array values
					values.push_back( oi->second);
					if (!copy->strar.empty()) copy->strar.push_back( '\0');
					copy->strar.append( oi->first);
				}
			}
			// Insert all key/values into new map
			typename std::vector<ValueType>::const_iterator vi = values.begin(), ve = values.end();
			char const* ki = copy->strar.c_str();
			for (; vi != ve; ++vi,ki=std::strchr( ki, '\0')+1)
			{
				copy->map.insert( typename Map::value_type( ki, *vi));
			}
			return copy;
		}

		template <class Updater>
		BlockKeyMapRef setValueAndGetUpdatedBlock( const char* key, ValueType value, Updater updater)
		{
			typename Map::iterator mi = map.find( key);
			if (mi == map.end())
			{
				BlockKeyMapRef copy = createBlockKeyMapRef();
				std::size_t keylen = std::strlen( key);

				copy->strar.reserve( keylen + 1 + strar.size());
				copy->strar.append( strar);
				copy->strar.push_back( '\0');
				copy->strar.append( key);
				const char* new_key = copy->strar.c_str() + copy->strar.size() - keylen;
				copy->map.insert( typename Map::value_type( new_key, value));
				typename Map::const_iterator oi = map.begin(), oe = map.end();
				for (; oi != oe; ++oi)
				{
					const char* o_key = copy->strar.c_str() + (oi->first - strar.c_str());
					copy->map.insert( typename Map::value_type( o_key, oi->second));
				}
				return copy;
			}
			else
			{
				updater( mi->second, value);
				return BlockKeyMapRef();
			}
		}

		const ValueType* find( const char* key) const
		{
			typename Map::const_iterator mi = map.find(key);
			return (mi == map.end()) ? NULL : &mi->second;
		}

		typedef typename Map::const_iterator const_iterator;
		const_iterator begin() const	{return map.begin();}
		const_iterator end() const	{return map.end();}
	};

private:
	static uint32_t hashString( const char* key)
	{
		return utils::Crc32::calc( key);
	}
	static uint32_t hashString( const std::string& key)
	{
		return utils::Crc32::calc( key.c_str(), key.size());
	}

	typedef std::vector<BlockKeyMapRef> BlockKeyMapRefArray;

public:
	class const_iterator
	{
	public:
		const_iterator( const const_iterator& o)
			:m_cur(o.m_cur),m_itr(o.m_itr),m_end(o.m_end),m_elemitr(o.m_elemitr){}
		const_iterator( typename BlockKeyMapRefArray::const_iterator itr_, typename BlockKeyMapRefArray::const_iterator end_)
			:m_itr(itr_),m_end(end_)
		{
			skip();
		}

		typedef std::pair<const char* const,ValueType> value_type;

		const value_type& operator* () const
		{
			return *m_elemitr;
		}
		const value_type* operator-> () const
		{
			return &*m_elemitr;
		}
		const_iterator& operator++() {next(); return *this;}
		const_iterator operator++(int) {const_iterator rt=*this; next(); return rt;}

		bool operator == (const const_iterator& o) const	{return isequal(o);}
		bool operator != (const const_iterator& o) const	{return !isequal(o);}

	private:
		void skip()
		{
			for (;;)
			{
				m_cur = BlockKeyMapRef();
				for (; m_itr != m_end; ++m_itr)
				{
					m_cur = *m_itr;
					if (m_cur.get()) break;
				}
				if (m_cur.get())
				{
					m_elemitr = m_cur->begin();
					if (m_elemitr == m_cur->end()) continue;
				}
				break;
			}
		}

		void next()
		{
			if (m_cur.get() && m_elemitr != m_cur->end())
			{
				++m_elemitr;
				if (m_elemitr == m_cur->end())
				{
					++m_itr;
					skip();
				}
			}
			else
			{
				++m_itr;
				skip();
			}
		}

		int isequal( const const_iterator& o) const
		{
			return isequal( *this, o);
		}

		static bool isequal( const const_iterator& aa, const const_iterator& bb)
		{
			if (!aa.m_cur.get()) return !bb.m_cur.get();
			if (!bb.m_cur.get()) return false;
			return (aa.m_itr == bb.m_itr && aa.m_elemitr == bb.m_elemitr);
		}

	private:
		BlockKeyMapRef m_cur;
		typename BlockKeyMapRefArray::const_iterator m_itr;
		typename BlockKeyMapRefArray::const_iterator m_end;
		typename BlockKeyMap::Map::const_iterator m_elemitr;
	};

	const_iterator begin() const
	{
		return const_iterator( m_blocks.begin(), m_blocks.end());
	}
	const_iterator end() const
	{
		return const_iterator( m_blocks.end(), m_blocks.end());
	}

private:
	strus::mutex m_writerMutex;
	uint32_t m_hashMask;
	int m_nofHashBits;
	BlockKeyMapRefArray m_blocks;
};

}//namespace
#endif


