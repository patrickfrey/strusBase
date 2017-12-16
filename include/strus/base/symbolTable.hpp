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
#include "strus/base/unordered_map.hpp"
#include <list>
#include <vector>
#include <string>
#include <cstring>
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

	typedef strus::unordered_map<Key,uint32_t,HashFunc,MapKeyEqual> Map;
	typedef strus::unordered_map<Key,uint32_t,HashFunc,MapKeyEqual>::const_iterator const_iterator;

public:
	///\brief Default constructor
	SymbolTable()
		:m_keystring_blocks(createKeystringBlocks()),m_isnew(false)

	{
		if (!m_keystring_blocks) throw std::bad_alloc();
	}
	///\brief Destructor
	~SymbolTable();

	///\brief Get handle ( >= 1) associated with key, create one if not defined
	///\param[in] key string
	///\return the handle for the key or 0 on a memory allocation error
	uint32_t getOrCreate( const std::string& key);
	///\brief Get handle ( >= 1) associated with key, create one if not defined
	///\param[in] key key string poiner
	///\param[in] keysize size of key in bytes
	///\return the handle for the key or 0 on a memory allocation error
	uint32_t getOrCreate( const char* key, std::size_t keysize);

	///\brief Get handle associated with key or 0 if not defined
	///\param[in] key string
	///\return the handle for the key or 0 if not defined
	uint32_t get( const std::string& key) const;
	///\brief Get handle associated with key or 0 if not defined
	///\param[in] key key string poiner
	///\param[in] keysize size of key in bytes
	///\return the handle for the key or 0 if not defined
	uint32_t get( const char* key, std::size_t keysize) const;

	///\brief Inverse lookup, get key of handle
	///\param[in] id key handle
	///\return the key string
	const char* key( const uint32_t& id) const;

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
	///\return the number of elements defined
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

	///\brief Evaluate if the last symbol retrieved with getOrCreate was new
	///\return true if the last symbol retrieved with getOrCreate was new
	bool isNew() const
	{
		return m_isnew;
	}

	///\brief Allocate a block in the context of the symbol table
	///\note Useful for building structures with symbols
	///\param[in] blocksize number of elements to allocate
	///\param[in] elemsize size of one element to allocate in bytes
	///\return the block allocated (not freed till end of symbol table life time)
	void* allocBlock( unsigned int blocksize, unsigned int elemsize);

private:
	SymbolTable( const SymbolTable&){}	///> non copyable
	void operator=( const SymbolTable&){}	///> non copyable

	static StringMapKeyBlockList* createKeystringBlocks();

private:
	Map m_map;
	std::vector<const char*> m_invmap;
	StringMapKeyBlockList* m_keystring_blocks;
	bool m_isnew;
};

}//namespace
#endif


