/*
 * Copyright (c) 2020 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
///\brief Get a string built by a format string with named elements
#ifndef _STRUS_BASE_STRING_NAMED_FORMAT_HPP_INCLUDED
#define _STRUS_BASE_STRING_NAMED_FORMAT_HPP_INCLUDED
#include <string>
#include <vector>
#include <map>
#include <cstdlib>

namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;

class NamedFormatString
{
public:
	/// \param[in] str string with name of substitutes in curly brackets '{' '}' with single bracket escaped by backslash '\'
	NamedFormatString( const std::string& str, ErrorBufferInterface* errorhnd);
	/// \param[in] str string with name of substitutes in curly brackets '{' '}' with single bracket escaped by backslash '\'
	NamedFormatString()
		:m_varmap(),m_ar(){}
	/// \brief Copy constructor
	NamedFormatString( const NamedFormatString& o)
		:m_varmap(o.m_varmap),m_ar(o.m_ar){}

	/// \param[in] name substitute variable name (case insensitive)
	/// \param[in] idx index assigned (not 0 as 0 is used for undefined value in Chunk) 
	/// \return true if there was a variable dound with that name, false else or if an error occurred
	bool assign( const std::string& name, int idx);

	class Chunk
	{
	public:
		const std::string& prefix() const	{return m_prefix;}
		int idx() const				{return m_idx;}

		Chunk( const std::string& prefix_, int idx_=0)
			:m_prefix(prefix_),m_idx(idx_){}
		Chunk( const Chunk& o)
			:m_prefix(o.m_prefix),m_idx(o.m_idx){}

		void setIndex( int idx_)		{m_idx = idx_;}

	private:
		std::string m_prefix;
		int m_idx;
	};

	typedef std::vector<Chunk>::const_iterator const_iterator;

	const_iterator begin() const	{return m_ar.begin();}
	const_iterator end() const	{return m_ar.end();}

private:
	std::multimap<std::string,std::size_t> m_varmap;
	std::vector<Chunk> m_ar;
};

} //namespace
#endif
