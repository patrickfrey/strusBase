/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
///\brief Map of strings to indices not freed till end of table life time.
#ifndef _STRUS_BASE_SYMBOL_TABLE_HPP_INCLUDED
#define _STRUS_BASE_SYMBOL_TABLE_HPP_INCLUDED
#include "strus/base/crc32.hpp"
#include <boost/unordered_map.hpp>
#include <list>
#include <vector>
#include <string>
#include <stdexcept>

namespace strus
{
///\brief Forward declaration
class StringMapKeyBlockList;

///\brief Map of strings to indices not freed till end of table life time
/// \note Suitable for symbol tables of languages (DSL)
class SymbolTable
{
private:
	///\brief Key of symbol table
	struct Key
	{
		const char* str;
		std::size_t len;

		Key()
			:str(0),len(0){}
		Key( const char* str_, std::size_t len_)
			:str(str_),len(len_){}
		Key( const Key& o)
			:str(o.str),len(o.len){}
	};
	struct MapKeyLess
	{
		bool operator()( const Key& a, const Key& b) const
		{
			if (a.len < b.len) return true;
			if (a.len > b.len) return false;
			return std::memcmp( a.str, b.str, a.len) < 0;
		}
	};
	struct MapKeyEqual
	{
		bool operator()( const Key& a, const Key& b) const
		{
			return a.len == b.len && std::memcmp( a.str, b.str, a.len) == 0;
		}
	};
	struct HashFunc{
		int operator()( const Key& key)const
		{
			return utils::Crc32::calc( key.str, key.len);
		}
	};

	typedef boost::unordered_map<Key,uint32_t,HashFunc,MapKeyEqual> Map;
	typedef boost::unordered_map<Key,uint32_t,HashFunc,MapKeyEqual>::const_iterator const_iterator;

public:
	///\brief Default constructor
	explicit SymbolTable()
		:m_keystring_blocks(createKeystringBlocks())

	{
		if (!m_keystring_blocks) throw std::bad_alloc();
	}
	~SymbolTable();

	///\brief Get handle ( >= 1) associated with key, create one if not defined
	uint32_t getOrCreate( const std::string& key);
	///\brief Get handle ( >= 1) associated with key, create one if not defined
	uint32_t getOrCreate( const char* key, std::size_t keysize);

	///\brief Get handle associated with key or 0 if not defined
	uint32_t get( const std::string& key) const;
	///\brief Get handle associated with key or 0 if not defined
	uint32_t get( const char* key, std::size_t keysize) const;

	///\brief Inverse lookup, get key of handle
	const char* key( const uint32_t& value) const;

	///\brief Get start iterator (unordered)
	const_iterator begin() const
	{
		return m_map.begin();
	}
	///\brief Get end iterator (unordered)
	const_iterator end() const
	{
		return m_map.end();
	}
	///\brief Get number of elements defined
	std::size_t size() const
	{
		return m_invmap.size();
	}

	typedef std::vector<const char*>::const_iterator const_inv_iterator;

	///\brief Get start iterator of inv
	const_inv_iterator inv_begin() const
	{
		return m_invmap.begin();
	}
	///\brief Get end iterator of inv
	const_inv_iterator inv_end() const
	{
		return m_invmap.end();
	}

	bool isNew( uint32_t id) const
	{
		return id == m_invmap.size();
	}

private:
	SymbolTable( const SymbolTable&){}	///> non copyable
	void operator=( const SymbolTable&){}	///> non copyable

	static StringMapKeyBlockList* createKeystringBlocks();

private:
	Map m_map;
	std::vector<const char*> m_invmap;
	StringMapKeyBlockList* m_keystring_blocks;
};

}//namespace
#endif


