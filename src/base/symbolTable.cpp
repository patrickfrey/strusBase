/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/symbolTable.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/unordered_map.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/internationalization.hpp"
#include <cstdlib>
#include <cstring>
#include <limits>

namespace strus {

/// \Brief Unit of allocation for the keys in the symbol table
class StringMapKeyBlock
{
public:
	/// \brief Default block size
	enum {DefaultSize = 16300};

public:
	/// \brief Default constructor
	explicit StringMapKeyBlock( std::size_t blksize_=DefaultSize, std::size_t elemsize_=1);
	/// \brief Copy constructor
	StringMapKeyBlock( const StringMapKeyBlock& o);
	/// \brief Destructor
	~StringMapKeyBlock();

	/// \brief Allocate a string in the block
	/// \return the immutable pointer to the key or 0, if the block does not have enough free space for the key to allocate
	const char* allocKey( const std::string& key);

	/// \brief Allocate a string in the block
	/// \return the immutable pointer to the key or 0, if the block does not have enough free space for the key to allocate
	const char* allocKey( const char* key, std::size_t keylen);

	void* blockPtr()
	{
		return m_blk;
	}

private:
	char* m_blk;
	std::size_t m_blksize;
	std::size_t m_blkpos;
};


/// \Brief List of allocation units for the keys in the symbol table
class StringMapKeyBlockList
{
public:
	/// \brief Default constructor
	StringMapKeyBlockList(){}
	/// \brief Copy constructor
	StringMapKeyBlockList( const StringMapKeyBlockList& o)
		:m_ar(o.m_ar){}

	/// \brief Allocate a key
	/// \return the immutable pointer to the key
	const char* allocKey( const char* key, std::size_t keylen);

	/// \brief Free all keys allocated
	void clear();

	void* allocBlock( std::size_t blksize_, std::size_t elemsize_);

private:
	std::list<StringMapKeyBlock> m_ar;
};

}//namespace

using namespace strus;

DLL_PUBLIC BlockAllocator::~BlockAllocator()
{
	if (m_blocks) delete m_blocks;
}

DLL_PUBLIC const char* BlockAllocator::allocStringCopy( const std::string& str)
{
	return allocStringCopy( str.c_str(), str.size());
}

DLL_PUBLIC const char* BlockAllocator::allocStringCopy( const char* str, std::size_t size)
{
	try
	{
		return m_blocks->allocKey( str, size);
	}
	catch (const std::exception&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
		return 0;
	}
}

DLL_PUBLIC StringMapKeyBlockList* BlockAllocator::createBlocks()
{
	try
	{
		return new StringMapKeyBlockList();
	}
	catch (const std::exception&)
	{
		return 0;
	}
}

DLL_PUBLIC void BlockAllocator::deleteBlocks( StringMapKeyBlockList* ptr)
{
	delete ptr;
}

StringMapKeyBlock::StringMapKeyBlock( std::size_t blksize_, std::size_t elemsize_)
	:m_blk((char*)std::calloc(blksize_,elemsize_)),m_blksize(blksize_*elemsize_),m_blkpos(0)
{
	if (!m_blk) throw std::bad_alloc();
}

StringMapKeyBlock::StringMapKeyBlock( const StringMapKeyBlock& o)
	:m_blk((char*)std::malloc(o.m_blksize)),m_blksize(o.m_blksize),m_blkpos(o.m_blkpos)
{
	if (!m_blk) throw std::bad_alloc();
	std::memcpy( m_blk, o.m_blk, o.m_blksize);
}

StringMapKeyBlock::~StringMapKeyBlock()
{
	std::free( m_blk);
}

const char* StringMapKeyBlock::allocKey( const std::string& key)
{
	const char* rt = m_blk + m_blkpos;
	if (key.size() > m_blksize || key.size() + m_blkpos + 1 > m_blksize) return 0;
	std::memcpy( m_blk + m_blkpos, key.c_str(), key.size()+1);
	m_blkpos += key.size()+1;
	return rt;
}

const char* StringMapKeyBlock::allocKey( const char* key, std::size_t keylen)
{
	const char* rt = m_blk + m_blkpos;
	if (keylen > m_blksize || keylen + m_blkpos + 1 > m_blksize) return 0;
	std::memcpy( m_blk + m_blkpos, key, keylen+1);
	m_blkpos += keylen+1;
	return rt;
}

void* StringMapKeyBlockList::allocBlock( std::size_t blksize_, std::size_t elemsize_)
{
	m_ar.push_front( StringMapKeyBlock( blksize_, elemsize_));
	return m_ar.front().blockPtr();
}

const char* StringMapKeyBlockList::allocKey( const char* key, std::size_t keylen)
{
	const char* rt;
	if (m_ar.empty())
	{
		if (keylen > StringMapKeyBlock::DefaultSize)
		{
			m_ar.push_front( StringMapKeyBlock( keylen+1));
			rt = m_ar.front().allocKey( key, keylen);
		}
		else
		{
			m_ar.push_back( StringMapKeyBlock());
			rt = m_ar.back().allocKey( key, keylen);
		}
	}
	else
	{
		rt = m_ar.back().allocKey( key, keylen);
		if (!rt)
		{
			if (keylen > StringMapKeyBlock::DefaultSize)
			{
				m_ar.push_front( StringMapKeyBlock( keylen+1));
				rt = m_ar.front().allocKey( key, keylen);
			}
			else
			{
				m_ar.push_back( StringMapKeyBlock());
				rt = m_ar.back().allocKey( key, keylen);
			}
		}
	}
	if (!rt) throw std::bad_alloc();
	return rt;
}


void StringMapKeyBlockList::clear()
{
	m_ar.clear();
}

namespace strus {
class InternalMap
	:public strus::unordered_map<SymbolTable::Key,uint32_t,SymbolTable::HashFunc,SymbolTable::MapKeyEqual>
{
public:
	InternalMap() :strus::unordered_map<SymbolTable::Key,uint32_t,SymbolTable::HashFunc,SymbolTable::MapKeyEqual>(){}
};
}

DLL_PUBLIC SymbolTable::~SymbolTable()
{
	delete m_map;
	delete m_keystring_blocks;
}

DLL_PUBLIC StringMapKeyBlockList* SymbolTable::createKeystringBlocks()
{
	try
	{
		return new StringMapKeyBlockList();
	}
	catch (const std::exception&)
	{
		return 0;
	}
}

DLL_PUBLIC void SymbolTable::deleteKeystringBlocks( StringMapKeyBlockList* ptr)
{
	delete ptr;
}

DLL_PUBLIC InternalMap* SymbolTable::createInternalMap()
{
	try
	{
		return new InternalMap();
	}
	catch (const std::exception&)
	{
		return 0;
	}
}

DLL_PUBLIC void SymbolTable::deleteInternalMap( InternalMap* ptr)
{
	delete ptr;
}

DLL_PUBLIC uint32_t SymbolTable::getOrCreate( const std::string& key_)
{
	return getOrCreate( key_.c_str(), key_.size());
}

DLL_PUBLIC uint32_t SymbolTable::getOrCreate( const char* keystr, std::size_t keylen)
{
	try
	{
		Key newkey( keystr, keylen);
		InternalMap::const_iterator itr = m_map->find( newkey);
		m_isnew = (itr == m_map->end());
		if (m_isnew)
		{
			if (m_invmap.size() >= (std::size_t)std::numeric_limits<int32_t>::max()-1)
			{
				throw std::bad_alloc();
			}
			const char* keystr_copy = m_keystring_blocks->allocKey( newkey.str, newkey.len);
			m_invmap.push_back( keystr_copy);
			Key kk( keystr_copy, newkey.len);
			(*m_map)[ kk] = m_invmap.size();
			return m_invmap.size();
		}
		else
		{
			return itr->second;
		}
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
		return 0;
	}
	catch (const std::exception& err)
	{
		m_errorhnd->report( ErrorCodeRuntimeError, "%s", err.what());
		return 0;
	}
}

DLL_PUBLIC uint32_t SymbolTable::get( const std::string& key_) const
{
	return get( key_.c_str(), key_.size());
}

DLL_PUBLIC uint32_t SymbolTable::get( const char* keystr, std::size_t keylen) const
{
	Key keystruct( keystr, keylen);
	InternalMap::const_iterator itr = m_map->find( keystruct);
	if (itr != m_map->end())
	{
		return itr->second;
	}
	return 0;
}

DLL_PUBLIC const char* SymbolTable::key( const uint32_t& value) const
{
	if (!value || value > (uint32_t)m_invmap.size()) return 0;
	return m_invmap[ value-1];
}

DLL_PUBLIC void* SymbolTable::allocBlock( unsigned int blocksize, unsigned int elemsize)
{
	return m_keystring_blocks->allocBlock( blocksize, elemsize);
}

