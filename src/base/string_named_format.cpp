/*
 * Copyright (c) 2020 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
///\brief Get a string built by a format string with named elements
#include "strus/base/string_named_format.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/string_conv.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/errorUtils.hpp"
#include <cstring>

using namespace strus;

static bool isAlphaNum( char ch)
{
	if (ch >= '0' && ch <= '9') return true;
	if ((ch|32) >= 'a' && (ch|32) <= 'z') return true;
	if (ch == '_') return true;
	return false;
}

static std::string parseIdentifier( char const*& ci, const std::string& alphabet)
{
	std::string rt;
	for (; *ci && (unsigned char)*ci <= 32; ++ci){}
	for (; isAlphaNum(*ci) || 0!=std::strchr( alphabet.c_str(), *ci) ; ++ci)
	{
		rt.push_back( *ci | 32);
	}
	for (; *ci && (unsigned char)*ci <= 32; ++ci){}
	return rt;
}

DLL_PUBLIC void NamedFormatString::init( const std::string& str, const std::string& alphabet, ErrorBufferInterface* errorhnd)
{
	try
	{
		char const* ci = str.c_str();
		
		std::string prefix;
		for (; *ci; ++ci)
		{
			if (*ci == '\\')
			{
				++ci;
				if (*ci == '{' || *ci == '}')
				{
					prefix.push_back( *ci);
				}
				else
				{
					prefix.push_back( '\\');
					prefix.push_back( *ci);
				}
			}
			else if (*ci == '{')
			{
				++ci;
				std::string name = parseIdentifier( ci, alphabet);
				m_varmap.insert( std::pair<std::string,std::size_t>( name, m_ar.size()));
				m_ar.push_back( prefix);
				prefix.clear();
				if (*ci != '}') throw std::runtime_error(_TXT("missing closing curly bracket at end of variable reference"));
			}
			else if (*ci == '}')
			{
				throw std::runtime_error( _TXT("unbalanced curly brackets in named format string"));
			}
			else
			{
				prefix.push_back( *ci);
			}
		}
		if (!prefix.empty())
		{
			m_ar.push_back( prefix);
		}
	}
	CATCH_ERROR_MAP( _TXT("error parsing named format string: %s"), *errorhnd);
}

DLL_PUBLIC NamedFormatString::NamedFormatString( const std::string& str, ErrorBufferInterface* errorhnd)
{
	init( str, "", errorhnd);
}

DLL_PUBLIC NamedFormatString::NamedFormatString( const std::string& str, const std::string& alphabet, ErrorBufferInterface* errorhnd)
{
	init( str, alphabet, errorhnd);
}

DLL_PUBLIC bool NamedFormatString::assign( const std::string& name, int idx)
{
	typedef std::multimap<std::string,std::size_t>::iterator mitr;
	std::pair<mitr,mitr> eq = m_varmap.equal_range( strus::string_conv::tolower( name));
	mitr mi = eq.first, me = eq.second;
	for (; mi != me; ++mi)
	{
		m_ar[ mi->second].setIndex( idx);
	}
	return eq.first != eq.second;
}

