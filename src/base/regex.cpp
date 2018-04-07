/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Regular expression matching (implementation based on boost)
#include "strus/base/regex.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/internationalization.hpp"
#include "strus/base/dll_tags.hpp"
#include "cxx11features.hpp"

#undef USE_STD_REGEX
#if __cplusplus >= 201103L && HAS_CXX11_REGEX != 0
#define USE_STD_REGEX
#endif // __cplusplus >= 201103L && HAS_CXX11_REGEX

#ifdef USE_STD_REGEX
#include <regex>
namespace rx = std;
#else
#include <boost/regex.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
namespace rx = boost;
#endif

using namespace strus;

class RegexOutputFormatter
{
public:
	RegexOutputFormatter()
		:m_items(),m_strings(){}

	RegexOutputFormatter( const RegexOutputFormatter& o)
		:m_items(o.m_items),m_strings(o.m_strings){}

	explicit RegexOutputFormatter( const std::string& fmt)
	{
		char const* fi = fmt.c_str();
		const char* pre = fi;
		while (*fi)
		{
			if (*fi == '$')
			{
				if (fi > pre)
				{
					int stridx = m_strings.size()+1;
					m_strings.push_back( '\0');
					m_strings.append( pre, fi - pre);
					if (stridx > std::numeric_limits<int>::max()) throw std::runtime_error( _TXT("output formatter string size out of range"));
					m_items.push_back( stridx);
				}
				++fi;
				int idx = 0;
				while (*fi >= '0' && *fi <= '9')
				{
					idx = idx * 10 + (*fi - '0');
					if (idx > std::numeric_limits<short>::max()) throw std::runtime_error( _TXT("output formatter element reference index out of range"));
					++fi;
				}
				m_items.push_back( -idx);
				pre = fi;
			}
			else
			{
				++fi;
			}
		}
		if (fi > pre)
		{
			int stridx = m_strings.size()+1;
			m_strings.push_back( '\0');
			m_strings.append( pre, fi - pre);
			if (stridx > std::numeric_limits<int>::max()) throw std::runtime_error( _TXT("output formatter string size out of range"));
			m_items.push_back( stridx);
		}
	}

	std::string print( const rx::smatch& match) const
	{
		std::string rt;
		std::vector<int>::const_iterator ti = m_items.begin(), te = m_items.end();
		for (; ti != te; ++ti)
		{
			if (*ti <= 0)
			{
				//... output variable
				rt.append( match.str( -*ti));
			}
			else
			{
				rt.append( m_strings.c_str() + *ti);
			}
		}
		return rt;
	}

private:
	std::vector<int> m_items;
	std::string m_strings;
};

#ifdef USE_STD_REGEX
#define REGEX_SYNTAX std::regex_constants::extended
#define MATCH_FLAGS std::regex_constants::match_default
#define MATCH_START_FLAGS std::regex_constants::match_continuous
#else
#define REGEX_SYNTAX boost::regex::extended
#define MATCH_FLAGS boost::match_posix
#define MATCH_START_FLAGS boost::regex_constants::match_continuous
#endif

struct RegexSearchConfiguration
{
	rx::regex expression;
	unsigned int index;

	RegexSearchConfiguration( const std::string& expressionstr, unsigned int index_)
		:expression(expressionstr),index(index_){}
	RegexSearchConfiguration( const RegexSearchConfiguration& o)
		:expression(o.expression),index(o.index){}
};

struct RegexSubstConfiguration
{
	rx::regex expression;
	RegexOutputFormatter formatter;

	RegexSubstConfiguration( const std::string& expressionstr, const std::string& fmtstr)
		:expression(expressionstr, REGEX_SYNTAX)
		,formatter(fmtstr){}
};

DLL_PUBLIC RegexSearch::RegexSearch( const std::string& expression, int index, ErrorBufferInterface* errhnd_)
	:m_config(0),m_errhnd(errhnd_)
{
	try
	{
		m_config = new RegexSearchConfiguration( expression, index);
	}
	catch (const std::exception& err)
	{
		m_errhnd->report( ErrorCodeInvalidRegex, _TXT("error in constructor of regex search: %s"), err.what());
	}
}

DLL_PUBLIC RegexSearch::~RegexSearch()
{
	if (m_config) delete (RegexSearchConfiguration*)m_config;
}

DLL_PUBLIC RegexSearch::Match RegexSearch::find( const char* start, const char* end) const
{
	try
	{
		if (m_config)
		{
			rx::match_results<char const*> pieces_match;
			char const* si = start;
			char const* se = end;
			if (rx::regex_search( si, se, pieces_match, ((RegexSearchConfiguration*)m_config)->expression, MATCH_FLAGS))
			{
				int idx = ((RegexSearchConfiguration*)m_config)->index;
				int mpos = pieces_match.position( idx);
				int mlen = pieces_match.length( idx);
				return RegexSearch::Match( mpos, mlen);
			}
		}
		return RegexSearch::Match();
	}
	catch (const std::exception& err)
	{
		m_errhnd->report( ErrorCodeRuntimeError, _TXT("error in regex search: %s"), err.what());
		return RegexSearch::Match();
	}
}

DLL_PUBLIC int RegexSearch::match_start( const char* start, const char* end) const
{
	try
	{
		if (m_config)
		{
			rx::match_results<char const*> match;
			char const* si = start;
			char const* se = end;
			if (rx::regex_search( si, se, match, ((RegexSearchConfiguration*)m_config)->expression, MATCH_START_FLAGS))
			{
				int idx = ((RegexSearchConfiguration*)m_config)->index;
				return match.length( idx);
			}
		}
		return -1;
	}
	catch (const std::exception& err)
	{
		m_errhnd->report( ErrorCodeRuntimeError, _TXT("error in regex search: %s"), err.what());
		return -1;
	}
}

DLL_PUBLIC RegexSubst::RegexSubst( const std::string& expression, const std::string& fmtstring, ErrorBufferInterface* errhnd_)
	:m_config(0),m_errhnd(errhnd_)
{
	try
	{
		m_config = new RegexSubstConfiguration( expression, fmtstring);
	}
	catch (const std::exception& err)
	{
		m_errhnd->report( ErrorCodeInvalidRegex, _TXT("error in constructor of regex subst: %s"), err.what());
	}
}

DLL_PUBLIC RegexSubst::~RegexSubst()
{
	if (m_config) delete (RegexSubstConfiguration*)m_config;
}

DLL_PUBLIC bool RegexSubst::exec( std::string& out, const std::string& input) const
{
	try
	{
		if (m_config)
		{
			rx::smatch pieces_match;

			if (rx::regex_match( input, pieces_match, ((RegexSubstConfiguration*)m_config)->expression))
			{
				out = ((RegexSubstConfiguration*)m_config)->formatter.print( pieces_match);
				return true;
			}
			else
			{
				out.clear();
				return true;
			}
		}
		return false;
	}
	catch (const std::exception& err)
	{
		m_errhnd->report( ErrorCodeRuntimeError, _TXT("error in regex subst: %s"), err.what());
		return false;
	}
}

DLL_PUBLIC bool RegexSubst::exec( std::string& out, const char* src, std::size_t srcsize) const
{
	return exec( out, std::string( src, srcsize));
}


