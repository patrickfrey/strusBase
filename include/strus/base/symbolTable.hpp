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
#include <list>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <new>

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;
///\brief Forward declaration
class StringMapKeyBlockList;
///\brief Forward declaration
class InternalMap;

class BlockAllocator
{
public:
	explicit BlockAllocator( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_blocks(createBlocks())
	{
		if (!m_blocks)
		{
			deleteBlocks( m_blocks);
			throw std::bad_alloc();
		}
	}
	~BlockAllocator();

	const char* allocStringCopy( const char* str, std::size_t size);
	const char* allocStringCopy( const std::string& str);

private:
	static StringMapKeyBlockList* createBlocks();
	static void deleteBlocks( StringMapKeyBlockList* ptr);

private:
	ErrorBufferInterface* m_errorhnd;
	StringMapKeyBlockList* m_blocks;
};


///\brief Map of strings to indices not freed till end of table life time
/// \note Suitable for symbol tables of languages (DSL)
class SymbolTable
{
public:
	///\brief Default constructor
	explicit SymbolTable( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_map(createInternalMap()),m_keystring_blocks(createKeystringBlocks()),m_isnew(false)
	{
		if (!m_keystring_blocks || !m_map)
		{
			if (!m_keystring_blocks) deleteKeystringBlocks( m_keystring_blocks);
			if (!m_map) deleteInternalMap( m_map);
			throw std::bad_alloc();
		}
	}

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
	struct HashFunc
	{
		int operator()( const Key& key)const
		{
			return utils::Crc32::calc( key.str, key.len);
		}
	};

public:
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

	///\brief Get number of elements defined
	///\return the number of elements defined
	std::size_t size() const
	{
		return m_invmap.size();
	}
	
	///\brief Evaluate if the symbol table is empty, without any definitions
	///\return true if yes
	bool empty() const
	{
		return m_invmap.empty();
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

	///\brief Free all keys allocated
	void clear();

private:
#if __cplusplus >= 201103L
	SymbolTable( const SymbolTable&) = delete;
	void operator=( const SymbolTable&) = delete;
#else
	SymbolTable( const SymbolTable&){}	///> non copyable
	void operator=( const SymbolTable&){}	///> non copyable
#endif
	static StringMapKeyBlockList* createKeystringBlocks();
	static void deleteKeystringBlocks( StringMapKeyBlockList* ptr);
	static InternalMap* createInternalMap();
	static void deleteInternalMap( InternalMap* ptr);

private:
	ErrorBufferInterface* m_errorhnd;
	InternalMap* m_map;
	std::vector<const char*> m_invmap;
	StringMapKeyBlockList* m_keystring_blocks;
	bool m_isnew;
};

}//namespace
#endif


