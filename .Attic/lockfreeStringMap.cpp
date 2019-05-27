/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of a map string to a type for shared read/write without locking for parallel read/write accesses
/// \file lockfreeStringMap.hpp
#include "strus/base/lockfreeStringMap.hpp"
#include "strus/base/utf8.hpp"
#include "strus/base/dll_tags.hpp"

using namespace strus;

static const char keyDelimiter = '\1';
static const char valDelimiter = '\2';

static char translateKeyChar( unsigned char ch)
{
	return (ch != keyDelimiter && ch != valDelimiter) ? ch : ' ';
}

static void translateKey( std::string& dest, const std::string& key)
{
	dest.reserve( dest.size() + key.size() + 2);
	dest.push_back( keyDelimiter);

	std::string::iterator ri = rt.begin(), re = rt.end();
	for (; ri != re; ++ri)
	{
		dest.push_back( translateKeyChar(*ri));
	}
	dest.push_back( valDelimiter);
}

static void translateIndex( std::string& dest, int idx)
{
	char idxstr[16];
	std::size_t idxstrsize = strus::utf8encode( idxstr, idx);
	dest.append( idxstr, idxstrsize);
}

DLL_PUBLIC bool LockfreeStringMapBase::appendKey( std::string& dest, const std::string& key, int idx)
{
	try
	{
		if (idx > MaxNofBlockKeys) return false;
		translateKey( dest, key);
		translateIndex( dest, idx);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

DLL_PUBLIC int LockfreeStringMapBase::findKey( const std::string& buf, const std::string& key)
{
	enum {MaxKeySize = 254};
	char keybuf[ MaxKeySize+2];
	std::size_t ki = 0, ke = key.size() >= MaxKeySize ? MaxKeySize : key.size();
	std::size_t di = 0;

	keybuf[ di++] = keyDelimiter;
	for (; ki<ke; ++ki)
	{
		keybuf[ di++] = translateKeyChar( key[ ki]);
	}
	bool partialKey = true;
	if (key.size() <= MaxKeySize)
	{
		partialKey = false;
		keybuf[ di++] = valDelimiter;
	}
	keybuf[ di++] = 0;
	char const* valptr = 0;

	if (partialKey)
	{
		char const* kitr = std::strstr( buf.c_str(), keybuf);
		while (kitr)
		{
			if (0==std::memcmp( kitr+1, key.c_str(), key.size()) && kitr[ key.size()+1] == valDelimiter)
			{
				valptr = kitr +  key.size() + 2;
				break;
			}
			else
			{
				kitr = std::strchr( kitr, valDelimiter);
				if (kitr)
				{
					kitr = std::strstr( kitr, keybuf);
				}
			}
		}
	}
	else
	{
		char const* kitr = std::strstr( buf.c_str(), keybuf);
		if (kitr)
		{
			valptr = kitr +  key.size() + 2;
		}
	}
	if (valptr)
	{
		return strus::utf8decode( valptr, strus::utf8charlen( *valptr));
	}
	else
	{
		return UnknownKey;
	}
}


DLL_PUBLIC uint64_t LockfreeStringMapBase::hashString( const char* key)
{
	uint64_t hashVal = 131;
	char const* kk = key;
	for (; *kk; ++kk)
	{
		hashVal = (hashVal << 4) + *(kk++) + ((hashVal >> 24) & 0xF);
	}
	return hashVal;
}
