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
#include "strus/base/regex.hpp"
#include <utility>
#include <string>
#include <vector>

namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Class for a lexem in a domain specific programming language with the following properties
///		- Indentiation is meaningless
///		- Spaces are only separating tokens and have no other meaning
///		- Only EOLN comments and only one type
///		- C strings (single or double quotes with backslash escaping and not crossing end of line)
///		- A fixed number of lexem types defined as regular expressions is known
class ProgramLexem
{
public:
	enum Type {Eof,Token,SQString,DQString,Error};
	Type type() const			{return m_type;}	// lexem type
	int id() const				{return m_id;}		// enumeration index in case of a token
	const std::string& value() const	{return m_value;}	// lexem value

	ProgramLexem( Type type_=Error)
		:m_type(type_),m_id(-1),m_value(){}
	ProgramLexem( const ProgramLexem& o)
		:m_type(o.m_type),m_id(o.m_id),m_value(o.m_value){}
	ProgramLexem( Type type_, int id_, const std::string& value_)
		:m_type(type_),m_id(id_),m_value(value_){}
	operator bool() const		{return !end();}

	bool end() const		{return m_type == Eof || m_type == Error;}
	bool isToken( int id_) const	{return m_type == Token && m_id == id_;}
	bool isToken() const		{return m_type == Token;}
	bool isString() const		{return m_type == SQString || m_type == DQString;}
	bool isError() const		{return m_type == Error;}
	bool isEof() const		{return m_type == Eof;}

private:
	Type m_type;			// lexem type
	int m_id;			// enumeration index in case of a token
	std::string m_value;		// lexem value
};

/// \brief Class for a lexer used for implementing domain specific languages of strus
class ProgramLexer
{
public:
	/// \brief Constructor
	/// \param[in] src pointer to source to parse
	/// \param[in] eolncomment_ operator used for comments (end of line comments)
	/// \param[in] errtokens illegal lexems defined as regular expressions (leading to a syntax error)
	/// \param[in] tokens lexems defined as regular expressions with the position in the array as associated id
	/// \param[in] errhnd_ error buffer interface to use for reporting errors
	ProgramLexer( const char* src, const char* eolncomment_, const char** tokens, const char** errtokens, ErrorBufferInterface* errhnd_);
	~ProgramLexer();

	/// \brief Get the next lexem
	const ProgramLexem& next();
	/// \brief Get the current lexem
	const ProgramLexem& current()		{return m_lexem;}
	/// \brief Rescan the current lexem (with different options)
	const ProgramLexem& rescanCurrent();

	enum Option {KeepStringEscaping=1};
	/// \brief Switch lexer option on/off
	/// \param[in] opt option identifier
	/// \param[in] value true=on, false=off
	/// \return previous option value
	bool setOption( Option opt_, bool value);

	/// \brief Get the current line number
	/// \remark expensive operation, if scans the source every time it is called, because it is thought to be used only for error messages
	int lineno() const;

	/// \brief Get the current source pointer
	const char* pos() const			{return m_src;}

	/// \brief Get the rest length to parse
	std::size_t len() const			{return m_end - m_src;}

	/// \brief Skip to a defined position is the parsed source
	/// \param[in] pos pointer to position to set scanner to
	/// \remark pos must be inside of the parsed source
	bool skipto( char const* pos);

	/// \brief Get the start of the next token
	/// \return the position of the next token or NULL, if an error occurred or if we got to the end of source
	const char* nextpos();

	/// \brief Get an excerpt of the current source location starting from a position with a marker inserted
	/// \param[in] posincr increment of the current position to show
	/// \param[in] marker marker to insert in the result at the current position
	/// \param[in] size size of excerpt to show
	std::string currentLocationString( int posincr, int size, const char* marker) const;

private:
#if __cplusplus >= 201103L
	ProgramLexer( const ProgramLexer&) = delete;
	void operator=( const ProgramLexer&) = delete;
#else
	ProgramLexer( const ProgramLexer&){}			///> non copyable
	void operator=( const ProgramLexer&){}			///> non copyable
#endif
	
private:
	ErrorBufferInterface* m_errhnd;
	const char* m_eolncomment;
	std::vector<strus::RegexSearch*> m_lexems;
	std::vector<strus::RegexSearch*> m_errlexems;
	const char* m_start;
	const char* m_end;
	char const* m_src;
	char const* m_prevsrc;
	ProgramLexem m_lexem;
	int m_opt;
};

} //namespace
#endif

