/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/symbolTable.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/utils.hpp"
#include <cstdlib>
#include <cstring>

using namespace strus;

DLL_PUBLIC StringMapKeyBlock::StringMapKeyBlock( std::size_t blksize_)
	:m_blk((char*)std::calloc(blksize_,1)),m_blksize(blksize_),m_blkpos(0)
{
	if (!m_blk) throw std::bad_alloc();
}

DLL_PUBLIC StringMapKeyBlock::StringMapKeyBlock( const StringMapKeyBlock& o)
	:m_blk((char*)std::malloc(o.m_blksize)),m_blksize(o.m_blksize),m_blkpos(o.m_blkpos)
{
	if (!m_blk) throw std::bad_alloc();
	std::memcpy( m_blk, o.m_blk, o.m_blksize);
}

DLL_PUBLIC StringMapKeyBlock::~StringMapKeyBlock()
{
	std::free( m_blk);
}

DLL_PUBLIC const char* StringMapKeyBlock::allocKey( const std::string& key)
{
	const char* rt = m_blk + m_blkpos;
	if (key.size() > m_blksize || key.size() + m_blkpos + 1 > m_blksize) return 0;
	std::memcpy( m_blk + m_blkpos, key.c_str(), key.size()+1);
	m_blkpos += key.size()+1;
	return rt;
}

DLL_PUBLIC const char* StringMapKeyBlock::allocKey( const char* key, std::size_t keylen)
{
	const char* rt = m_blk + m_blkpos;
	if (keylen > m_blksize || keylen + m_blkpos + 1 > m_blksize) return 0;
	std::memcpy( m_blk + m_blkpos, key, keylen+1);
	m_blkpos += keylen+1;
	return rt;
}

DLL_PUBLIC const char* StringMapKeyBlockList::allocKey( const char* key, std::size_t keylen)
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


DLL_PUBLIC void StringMapKeyBlockList::clear()
{
	m_ar.clear();
}


DLL_PUBLIC uint32_t SymbolTable::getOrCreate( const std::string& key)
{
	return getOrCreate( key.c_str(), key.size());
}

DLL_PUBLIC uint32_t SymbolTable::getOrCreate( const char* keystr, std::size_t keylen)
{
	Key key( keystr, keylen);
	Map::const_iterator itr = m_map.find( key);
	if (itr == m_map.end())
	{
		if (m_invmap.size() >= (std::size_t)std::numeric_limits<int32_t>::max()-1)
		{
			throw std::bad_alloc();
		}
		m_invmap.push_back( m_keystring_blocks.allocKey( key.str, key.len));
		m_map[ Key( m_invmap.back(), key.len)] = m_invmap.size();
		return m_invmap.size();
	}
	else
	{
		return itr->second;
	}
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


