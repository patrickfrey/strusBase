/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Class for a lexer used for implementing domain specific languages of strus
#include "strus/base/programLexer.hpp"
#include "private/internationalization.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/errorUtils.hpp"
#include "strus/base/dll_tags.hpp"
#include <cstring>

using namespace strus;

DLL_PUBLIC ProgramLexer::ProgramLexer( const char* src_, const char* eolncomment_, const char** tokens, const char** errtokens, ErrorBufferInterface* errhnd_)
	:m_errhnd(errhnd_),m_eolncomment(eolncomment_?eolncomment_:"")
	,m_lexems(),m_errlexems()
	,m_start(src_)
	,m_end(src_+std::strlen(src_))
	,m_src(src_)
	,m_lexem( ProgramLexem::Error)
{
	try
	{
		char const** ti = tokens;
		for (; *ti; ++ti)
		{
			m_lexems.push_back( new strus::RegexSearch( *ti, 0, m_errhnd));
			if (m_errhnd->hasError()) return;
		}
		ti = errtokens;
		for (; *ti; ++ti)
		{
			m_errlexems.push_back( new strus::RegexSearch( *ti, 0, m_errhnd));
			if (m_errhnd->hasError()) return;
		}
	}
	CATCH_ERROR_MAP( _TXT("error in lexer definition: %s"), *m_errhnd);
}

DLL_PUBLIC ProgramLexer::~ProgramLexer()
{
	std::vector<strus::RegexSearch*>::const_iterator li = m_lexems.begin(), le = m_lexems.end();
	for (;li != le; ++li) {delete *li;}
	li = m_errlexems.begin(), le = m_errlexems.end();
	for (;li != le; ++li) {delete *li;}
}

static inline bool isSpace( char ch)
{
	return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

static inline void skipToEoln( char const*& src)
{
	while (*src && *src != '\n')
	{
		if (*src == '\r' && src[1] != '\n') break;
		++src;
	}
}

static inline void skipSpaces( char const*& src, const char* eolcomment)
{
	for (;;)
	{
		while (isSpace( *src)) ++src;
		if (!*src) return;
		if (*src == eolcomment[0])
		{
			char const* cc = src;
			char const* ee = eolcomment;
			while (*cc && *ee == *cc) {++ee;++cc;}
			if (!*ee)
			{
				src = cc;
				skipToEoln( src);
				++src;
				continue;
			}
		}
		break;
	}
}

static std::string parse_STRING( char const*& src)
{
	std::string rt;
	char eb = *src++;
	while (*src != eb)
	{
		if (*src == '\0' || *src == '\n' || *src == '\r') throw strus::runtime_error( "%s", _TXT("unterminated string"));
		if (*src == '\\')
		{
			src++;
			if (*src == '\0' || *src == '\n' || *src == '\r') throw strus::runtime_error( "%s", _TXT("unterminated string"));
			if (*src == 'n')
			{
				rt.push_back( '\n');
			}
			else if (*src == 'b')
			{
				rt.push_back( '\b');
			}
			else if (*src == 'r')
			{
				rt.push_back( '\r');
			}
			else if (*src == 't')
			{
				rt.push_back( '\t');
			}
			else
			{
				rt.push_back( *src++);
			}
		}
		else
		{
			rt.push_back( *src++);
		}
	}
	++src;
	return rt;
}

static int findLexemInList( const std::vector<strus::RegexSearch*> lxlist, const char* src, const char* end, int& len)
{
	std::vector<strus::RegexSearch*>::const_iterator li = lxlist.begin(), le = lxlist.end();
	for (int lidx=0; li != le; ++li,++lidx)
	{
		int matchlen = (*li)->match_start( src, end);
		if (matchlen > 0)
		{
			len = matchlen;
			return lidx;
		}
	}
	return -1;
}

static std::string escString( const char* str, std::size_t len)
{
	std::string rt;
	char const* si = str;
	const char* se = str + len;
	for (; si != se && *si; ++si)
	{
		if ((unsigned char)*si < 32)
		{
			rt.push_back( ' ');
		}
		else
		{
			rt.push_back( *si);
		}
	}
	return rt;
}

DLL_PUBLIC std::string ProgramLexer::currentLocationString( int posincr, int size, const char* marker) const
{
	try
	{
		std::string rt;
		char const* si = m_src + posincr;
		enum {B10000000 = 128, B11000000 = 128 + 64};
		if (si < m_start)
		{
			si = m_start;
		}
		while (si < m_end && (*si & B11000000) == B10000000) ++si;
		for (int sidx=0; si < m_end && sidx < size; ++si,++sidx)
		{
			char const* chrend = si+1;
			while (chrend < m_end && (*chrend & B11000000) == B10000000) ++chrend;
			if (si <= m_src && chrend > m_src)
			{
				rt.append( marker);
			}
			rt.append( si, chrend-si);
		}
		return escString( rt.c_str(), rt.size());
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in lexer get current location string: %s"), *m_errhnd, std::string());
}

DLL_PUBLIC const ProgramLexem& ProgramLexer::next()
{
	try
	{
		if (m_errhnd->hasError()) return m_lexem = ProgramLexem( ProgramLexem::Error);
		skipSpaces( m_src, m_eolncomment);
		if (!*m_src) return m_lexem = ProgramLexem( ProgramLexem::Eof);
		if (*m_src == '\'')
		{
			return m_lexem = ProgramLexem( ProgramLexem::SQString, 0, parse_STRING( m_src));
		}
		else if (*m_src == '\"')
		{
			return m_lexem = ProgramLexem( ProgramLexem::DQString, 0, parse_STRING( m_src));
		}
		int len;
		int lidx = findLexemInList( m_lexems, m_src, m_end, len);
		if (lidx >= 0)
		{
			int errlen;
			if (0<=findLexemInList( m_errlexems, m_src, m_end, errlen) && errlen > len)
			{
				std::string errpos = escString( m_src, 30);
				m_errhnd->report( ErrorCodeSyntax, _TXT("syntax error on line %d of source, bad token at '%s..'"), lineno(), errpos.c_str());
				return m_lexem = ProgramLexem( ProgramLexem::Error);
			}
			else
			{
				char const* tok = m_src;
				m_src += len;
				return m_lexem = ProgramLexem( ProgramLexem::Token, lidx, std::string( tok, len));
			}
		}
		std::string errpos = escString( m_src, 30);
		m_errhnd->report( ErrorCodeSyntax, _TXT("syntax error on line %d of source, unknown token at '%s..'"), lineno(), errpos.c_str());
		return m_lexem = ProgramLexem( ProgramLexem::Error);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in lexer get next: %s"), *m_errhnd, m_lexem = ProgramLexem( ProgramLexem::Error));
}

DLL_PUBLIC const char* ProgramLexer::nextpos()
{
	if (m_errhnd->hasError()) return NULL;
	skipSpaces( m_src, m_eolncomment);
	return *m_src ? m_src : NULL;
}

DLL_PUBLIC bool ProgramLexer::skipto( char const* pos_)
{
	if (m_src > pos_ || m_end < pos_)
	{
		m_errhnd->report( ErrorCodeLogicError, _TXT("skip to illegal position in source"));
		return false;
	}
	m_src = pos_;
	return true;
}

DLL_PUBLIC int ProgramLexer::lineno() const
{
	char const* si = m_start;
	int linecnt = 0;
	for (; si != m_src; ++si)
	{
		if (*si == '\n') ++linecnt;
	}
	return linecnt + 1;
}

