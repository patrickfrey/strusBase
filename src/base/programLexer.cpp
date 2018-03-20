/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Class for a lexer used for implementing domain specific languages of strus
#include "strus/base/programLexer.hpp";
#include "private/internationalization.hpp"
#include "strus/errorBufferInterface.hpp"
#include <cstring>

using namespace strus;

ProgramLexer::ProgramLexer( const std::string& src, const char* eolncomment_, const char** operators_, ErrorBufferInterface* errhnd_)
	:m_errhnd(errhnd_),m_eolncomment(eolncomment_?eolncomment_:""),m_eolncommentlen(eolncomment_?std::strlen(eolncomment_):0),m_operators(operators_),m_start(src.c_str()),m_src(src.c_str()),m_len(src.size()){}

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
		if (*src == eolcomment[0])
		{
			char const* cc = src;
			char const* ee = eolcomment;
			while (*ee == *cc) {++ee;++cc;}
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

ProgramLexem ProgramLexer::next()
{
	skipSpaces( m_src, m_eolcomment)
	
}

bool ProgramLexer::skipto( char const* pos_)
{
	if (m_src > pos_ || m_start + m_len < pos_)
	{
		m_errhnd->report( ErrorCodeLogicError, _TXT("skip to illegal position in source"));
		return false;
	}
	m_src = pos_;
}

int ProgramLexer::lineno() const
{
	char const* si = m_start;
	int linecnt = 0;
	for (; si != m_src; ++si)
	{
		if (*si == '\n') ++linecnt;
	}
	return linecnt + 1;
}

