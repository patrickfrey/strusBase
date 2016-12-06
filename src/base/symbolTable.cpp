/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/symbolTable.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/utils.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include <cstdlib>
#include <cstring>

namespace strus {

/// \Brief Unit of allocation for the keys in the symbol table
class StringMapKeyBlock
{
public:
	/// \brief Default block size
	enum {DefaultSize = 16300};

public:
	/// \brief Default constructor
	explicit StringMapKeyBlock( std::size_t blksize_=DefaultSize);
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

private:
	std::list<StringMapKeyBlock> m_ar;
};

}//namespace

using namespace strus;

StringMapKeyBlock::StringMapKeyBlock( std::size_t blksize_)
	:m_blk((char*)std::calloc(blksize_,1)),m_blksize(blksize_),m_blkpos(0)
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

DLL_PUBLIC SymbolTable::~SymbolTable()
{
	delete m_keystring_blocks;
}

StringMapKeyBlockList* SymbolTable::createKeystringBlocks()
{
	return new StringMapKeyBlockList();
}

DLL_PUBLIC uint32_t SymbolTable::getOrCreate( const std::string& key)
{
	return getOrCreate( key.c_str(), key.size());
}

DLL_PUBLIC uint32_t SymbolTable::getOrCreate( const char* keystr, std::size_t keylen)
{
	try
	{
		Key key( keystr, keylen);
		Map::const_iterator itr = m_map.find( key);
		if (itr == m_map.end())
		{
			if (m_invmap.size() >= (std::size_t)std::numeric_limits<int32_t>::max()-1)
			{
				throw std::bad_alloc();
			}
			m_invmap.push_back( m_keystring_blocks->allocKey( key.str, key.len));
			m_map[ Key( m_invmap.back(), key.len)] = m_invmap.size();
			return m_invmap.size();
		}
		else
		{
			return itr->second;
		}
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating symbol: %s"), *m_errorhnd, 0);
}

DLL_PUBLIC uint32_t SymbolTable::get( const std::string& key) const
{
	return get( key.c_str(), key.size());
}

DLL_PUBLIC uint32_t SymbolTable::get( const char* keystr, std::size_t keylen) const
{
	Key key( keystr, keylen);
	Map::const_iterator itr = m_map.find( key);
	if (itr != m_map.end())
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


