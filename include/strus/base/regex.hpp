/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Regular expression matching
#ifndef _STRUS_BASE_REGEX_HPP_INCLUDED
#define _STRUS_BASE_REGEX_HPP_INCLUDED
#include <string>

namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;

/// \class RegexSearch
/// \brief Class to search for a regular expression
class RegexSearch
{
public:
	struct Match
	{
		int pos;	///< offset of the match in bytes from the start of the search or -1 if not found
		int len;	///< length of the match in bytes

		Match()
			:pos(-1),len(0){}
		Match( int pos_, int len_)
			:pos(pos_),len(len_){}
		Match( const Match& o)
			:pos(o.pos),len(o.len){}
		Match& operator=( const Match& o)
			{pos=o.pos;len=o.len; return *this;}
		bool valid() const	{return pos>=0;}
	};

	///\brief Constructor
	///\param[in] expression regular expression string (POSIX)
	RegexSearch( const std::string& expression, int index, ErrorBufferInterface* errhnd_);

	///\brief Destructor
	~RegexSearch();

	///\brief Execute the search
	///\param[in] begin pointer to the start of the string to search
	///\param[in] end pointer to the end of the string to search
	///\return a description of the match
	Match find( const char* begin, const char* end) const;

	///\brief Match the regex to the start of the source
	///\param[in] begin pointer to the start of the string to search
	///\param[in] end pointer to the end of the string to search
	///\return length of the match in byte or -1 if it did not match
	int match_start( const char* begin, const char* end) const;

private:
#if __cplusplus >= 201103L
	RegexSearch( const RegexSearch&) = delete;
	void operator=( const RegexSearch&) = delete;
#else
	RegexSearch( const RegexSearch&){}			///> non copyable
	void operator=( const RegexSearch&){}			///> non copyable
#endif

private:
	void* m_config;						//< PIMPL for implementation
	ErrorBufferInterface* m_errhnd;				//< where to report errors
};


/// \class RegexSubst
/// \brief Class to replace a regular expression by a string with sub-matches as substitute
class RegexSubst
{
public:
	///\brief Constructor
	///\param[in] expression regular expression string (POSIX)
	///\param[in] fmtstring format string for output with '$0' ... '$9' as substitutes for the results
	RegexSubst( const std::string& expression, const std::string& fmtstring, ErrorBufferInterface* errhnd_);
	///\brief Destructor
	~RegexSubst();

	///\brief Execute the replacement
	///\param[out] out output string
	///\param[in] input input string
	///\return bool true on success, false on failure, error code or message is not provided
	bool exec( std::string& out, const std::string& input) const;

	///\brief Execute the replacement
	///\param[out] out output string
	///\param[in] src pointer to input string
	///\param[in] srcsize sizeof src in bytes
	///\return bool true on success, false on failure, error code or message is not provided
	bool exec( std::string& out, const char* src, std::size_t srcsize) const;

	///\param[in] Get the last error in the constructor
	const char* error() const;

private:
#if __cplusplus >= 201103L
	RegexSubst( const RegexSubst&) = delete;
	void operator=( const RegexSubst&) = delete;
#else
	RegexSubst( const RegexSubst&){}			///> non copyable
	void operator=( const RegexSubst&){}			///> non copyable
#endif

private:
	void reportError( const char* err);

	void* m_config;						//< PIMPL for implementation
	ErrorBufferInterface* m_errhnd;				//< where to report errors
};

} //namespace
#endif

