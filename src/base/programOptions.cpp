/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/programOptions.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include "strus/base/string_format.hpp"
#include "strus/base/numstring.hpp"
#include "strus/base/dll_tags.hpp"
#include <cstring>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <cstdarg>

using namespace strus;

void ProgramOptions::OptMapDef::add( const char* arg)
{
	try
	{
		OptType optType = SingleBool;
		char alias = '\0';
		char const* aa = arg;
		const char* longnamestart = arg;
		const char* longnameend = arg + std::strlen(arg);
		for (;*aa;++aa)
		{
			if (*aa == ',')
			{
				if (aa - arg != 1)
				{
					throw strus::runtime_error( "%s",  _TXT("one character option expected before comma ',' in option definition string"));
				}
				alias = *arg;
				longnamestart = aa+1;
			}
			if (*aa == ':')
			{
				if (longnameend - aa != 1)
				{
					throw strus::runtime_error( "%s",  _TXT("colon ':' expected only at end of option definition string"));
				}
				longnameend = aa;
				optType = WithArgument;
			}
			else if (*aa == '+')
			{
				if (longnameend - aa != 1)
				{
					throw strus::runtime_error( "%s",  _TXT("plus '+' expected only at end of option definition string"));
				}
				longnameend = aa;
				optType = RepeatingBool;
			}

		}
		std::string longname( longnamestart, longnameend-longnamestart);
		if (longname.empty())
		{
			if (!alias)
			{
				throw strus::runtime_error( "%s",  _TXT("empty option definition"));
			}
			longname.push_back( alias);
		}
		if (alias)
		{
			m_aliasmap[ alias] = longname;
		}
		m_longnamemap[ longname] = optType;
	}
	CATCH_ERROR_MAP( _TXT("failed to define program option: %s"), *m_errorhnd);
}

bool ProgramOptions::OptMapDef::getOpt( const char* argv, std::vector<std::string>& optlist, std::string& optarg)
{
	try
	{
		optlist.clear();
		optarg.clear();
	
		if (argv[0] != '-' || argv[1] == '\0') return false;
	
		if (argv[1] == '-')
		{
			const char* oo = argv+2;
			const char* aa = std::strchr( oo, '=');
			if (aa)
			{
				optlist.push_back( std::string( oo, aa-oo));
				optarg = std::string( aa+1);
			}
			else
			{
				optlist.push_back( std::string( oo));
			}
		}
		else
		{
			const char* oo = argv+1;
			for (;*oo; ++oo)
			{
				std::map<char,std::string>::const_iterator oi = m_aliasmap.find( *oo);
				if (oi == m_aliasmap.end())
				{
					if (oo == argv+1) throw strus::runtime_error( _TXT("unknown option '-%c'"), *oo);
					optarg = std::string( oo);
					break;
				}
				optlist.push_back( std::string( oi->second));
			}
		}
		return true;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to read program option: %s"), *m_errorhnd, false);
}

DLL_PUBLIC ProgramOptions::ProgramOptions( ErrorBufferInterface* errorhnd_, int argc_, const char** argv_, int nofopt, ...)
	:m_errorhnd(errorhnd_),m_argc(argc_-1),m_argv(argv_+1)
{
	try
	{
		//[1] Initialize options map:
		OptMapDef optmapdef( m_errorhnd);
		va_list ap;
		va_start( ap, nofopt);
	
		for (int ai=0; ai<nofopt; ai++)
		{
			const char* av = va_arg( ap, const char*);
			optmapdef.add( av);
		}
		va_end(ap);
	
		//[2] Parse options and fill m_opt:
		std::vector<std::string> optlist;
		std::string optarg;
	
		for (; m_argc && optmapdef.getOpt( *m_argv, optlist, optarg); ++m_argv,--m_argc)
		{
			std::vector<std::string>::const_iterator oi = optlist.begin(), oe = optlist.end();
			for (; oi != oe; ++oi)
			{
				std::map<std::string,OptType>::iterator li = optmapdef.m_longnamemap.find( *oi);
				if (li == optmapdef.m_longnamemap.end()) throw strus::runtime_error( _TXT("unknown option '--%s'"), oi->c_str());
				if (li->second == WithArgument && oi+1 == oe)
				{
					if (optarg.empty() && m_argc > 1 && (m_argv[1][0] != '-' || m_argv[1][1] == '\0'))
					{
						if (m_argv[1][0] == '=')
						{
							if (!m_argv[1][1] && m_argc > 2)
							{
								--m_argc;
								++m_argv;
								m_opt.insert( OptMapElem( *oi, std::string( m_argv[1])));
							}
							else
							{
								m_opt.insert( OptMapElem( *oi, std::string( m_argv[1]+1)));
							}
						}
						else
						{
							m_opt.insert( OptMapElem( *oi, std::string( m_argv[1])));
						}
						--m_argc;
						++m_argv;
					}
					else
					{
						m_opt.insert( OptMapElem( *oi, optarg));
					}
				}
				else if (li->second == RepeatingBool)
				{
					OptMap::iterator mi = m_opt.find( *oi);
					if (mi == m_opt.end())
					{
						m_opt.insert( OptMapElem( *oi, "1"));
					}
					else
					{
						mi->second = strus::string_format( "%d", atoi( mi->second.c_str())+1);
					}
				}
				else
				{
					m_opt.insert( OptMapElem( *oi, std::string()));
				}
			}
		}
	}
	CATCH_ERROR_MAP( _TXT("failed to initialize program options structures: %s"), *m_errorhnd);
}

DLL_PUBLIC bool ProgramOptions::operator()( const std::string& optname) const
{
	try
	{
		return (m_opt.find( optname) != m_opt.end());
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to inspect program options: %s"), *m_errorhnd, false);
}

DLL_PUBLIC bool ProgramOptions::operator()( const char* optname) const
{
	try
	{
		return (m_opt.find( optname) != m_opt.end());
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to inspect program options: %s"), *m_errorhnd, false);
}

DLL_PUBLIC const char* ProgramOptions::operator[]( std::size_t idx) const
{
	try
	{
		if (idx >= m_argc) return 0;
		return m_argv[ idx];
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to inspect program options: %s"), *m_errorhnd, 0);
}

DLL_PUBLIC const char* ProgramOptions::operator[]( const std::string& optname) const
{
	try
	{
		if (m_opt.count( optname) > 1)
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, _TXT("option '%s' specified more than once"), optname.c_str());
		}
		std::map<std::string,std::string>::const_iterator oi = m_opt.find( optname);
		if (oi == m_opt.end()) return 0;
		return oi->second.c_str();
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to inspect program options: %s"), *m_errorhnd, 0);
}

DLL_PUBLIC int ProgramOptions::asInt( const std::string& optname) const
{
	try
	{
		if (m_opt.count( optname) > 1)
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, _TXT("option '%s' specified more than once"), optname.c_str());
		}
		std::map<std::string,std::string>::const_iterator oi = m_opt.find( optname);
		if (oi == m_opt.end()) return 0;
		try
		{
			return numstring_conv::toint( oi->second, std::numeric_limits<int>::max());
		}
		catch (const std::runtime_error&)
		{
			throw strus::runtime_error( _TXT("option '%s' has not the requested value type"), optname.c_str());
		}
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to inspect program options: %s"), *m_errorhnd, 0);
}

DLL_PUBLIC unsigned int ProgramOptions::asUint( const std::string& optname) const
{
	try
	{
		int rt = asInt( optname);
		if (rt < 0) throw strus::runtime_error( _TXT("non negative value expected for option '%s'"), optname.c_str());
		return (unsigned int)rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to inspect program options: %s"), *m_errorhnd, 0);
}

DLL_PUBLIC double ProgramOptions::asDouble( const std::string& optname) const
{
	try
	{
		if (m_opt.count( optname) > 1)
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, _TXT("option '%s' specified more than once"), optname.c_str());
		}
		std::map<std::string,std::string>::const_iterator oi = m_opt.find( optname);
		if (oi == m_opt.end()) return 0;
		try
		{
			return numstring_conv::todouble( oi->second);
		}
		catch (const std::runtime_error&)
		{
			throw strus::runtime_error( _TXT("option '%s' has not the requested value type %s"), optname.c_str(), "float/double");
		}
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to inspect program options: %s"), *m_errorhnd, 0);
}

DLL_PUBLIC std::vector<std::string> ProgramOptions::list( const std::string& optname) const
{
	try
	{
		std::vector<std::string> rt;
		std::pair<OptMap::const_iterator,OptMap::const_iterator> range = m_opt.equal_range( optname);
		OptMap::const_iterator ei = range.first, ee = range.second;
		for (; ei != ee; ++ei)
		{
			rt.push_back( ei->second);
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to inspect program options: %s"), *m_errorhnd, std::vector<std::string>());
}

DLL_PUBLIC int ProgramOptions::nofargs() const
{
	return m_argc;
}

DLL_PUBLIC const char** ProgramOptions::argv() const
{
	return m_argv;
}

DLL_PUBLIC void ProgramOptions::print( std::ostream& out)
{
	try
	{
		std::map<std::string,std::string>::const_iterator oi = m_opt.begin(), oe = m_opt.end();
		for (; oi != oe; ++oi)
		{
			out << "--" << oi->first << "=" << oi->second << std::endl;
		}
		std::size_t ai = 0, ae = m_argc;
		for (; ai != ae; ++ai)
		{
			out << "[" << ai << "] " << m_argv[ai] << std::endl;
		}
	}
	CATCH_ERROR_MAP( _TXT("failed to print program options: %s"), *m_errorhnd);
}

DLL_PUBLIC std::pair<const char*,const char*> ProgramOptions:: conflictingOpts( int nn, ...)
{
	const char* first = NULL;
	const char* second = NULL;
	va_list ap;
	va_start( ap, nn); 
	for (int ii = 0; ii < nn; ++ii)
	{
		if (!first)
		{
			first = va_arg( ap, const char*);
		}
		else if (!second)
		{
			second = va_arg( ap, const char*);
			if (second) break;
		}
	}
	va_end(ap);
	if (second)
	{
		return std::pair<const char*,const char*>( first, second);
	}
	else
	{
		return std::pair<const char*,const char*>( 0, 0);
	}
}


