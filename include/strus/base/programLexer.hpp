/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Class for a lexer used for implementing domain specific languages of strus
#ifndef _STRUS_BASE_PROGRAM_LEXER_HPP_INCLUDED
#define _STRUS_BASE_PROGRAM_LEXER_HPP_INCLUDED
#include <utility>
#include <string>

namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Class for a lexem in a domain specific programming language with the following properties
///		- Indentiation is meaningless
///		- Spaces are only separating tokens and have no other meaning
///		- Only EOLN comments and only one type
///		- C strings (single or double quotes with backslash escaping and not crossing end of line)
///		- A fixed number of operators is known
///		- Other tokens are alphanumeric identifiers, floating point numbers or integers
class ProgramLexem
{
	enum Type {Eof,Identifier,Integer,Float,SQString,DQString,Operator,Error};
	Type type;			// lexem type
	int id;				// enumeration index in case of an operator
	std::string value;		// lexem value

	ProgramLexem( const ProgramLexem& o)
		:type(o.type),id(o.id),value(o.value){}
	ProgramLexem( Type type_, int id_, const std::string& value_)
		:type(type_),id(id_),value(value_){}
};

/// \brief Class for a lexer used for implementing domain specific languages of strus
class ProgramLexer
{
public:
	ProgramLexer( const std::string& src, const char* eolncomment_, const char** tokens_, const char** operators_, ErrorBufferInterface* errhnd_);

	ProgramLexem next();

	int lineno() const;
	const char* pos() const			{return m_src;}
	std::size_t len() const			{return m_len - (m_src - m_start);}
	bool skipto( char const* pos_)		{if (m_src > pos_ || m_start + m_len < pos_) return false; m_src = pos_;}

private:
	ErrorBufferInterface* m_errhnd;
	const char* m_eolncomment;
	std::size_t m_eolncommentlen;
	const char** m_operators;
	const char* m_start;
	char const* m_src;
	std::size_t m_len;
};

