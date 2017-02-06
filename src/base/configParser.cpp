/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/configParser.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/utils.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include <map>
#include <cstring>
#include <stdexcept>

using namespace strus;

DLL_PUBLIC bool strus::extractStringFromConfigString( std::string& res, std::string& config, const char* key, ErrorBufferInterface* errorhnd)
{
	try
	{
		char const* cc = config.c_str();
		while (*cc)
		{
			while (*cc && ((unsigned char)*cc <= 32 || *cc == ';')) ++cc;
			//... skip spaces
			if (!*cc) break;

			const char* start = cc;
			std::string cfgkey;
			while (((*cc|32) >= 'a' && (*cc|32) <= 'z') || *cc == '_' || (*cc >= '0' && *cc <= '9'))
			{
				cfgkey.push_back( *cc++);
			}
			if (cfgkey.empty())
			{
				throw strus::runtime_error( _TXT( "expected item identifier as start of a declaration in a config string ('%s' | '%s')"), cfgkey.c_str(), config.c_str());
			}
			if (*cc != '=')
			{
				throw strus::runtime_error( _TXT( "'=' expected after item identifier in a config string ('%s %s' | '%s')"), cfgkey.c_str(), cc, config.c_str());
			}
			++cc;
			while (*cc && (unsigned char)*cc <= 32) ++cc;
			const char* nextItem;
			const char* endItem;
			if (*cc == '"' || *cc == '\'')
			{
				// Value is a string (without any escaping of characters supported):
				char eb = *cc++;
				for (endItem=cc; *endItem != '\0' && *endItem != eb; ++endItem){}
				if (*endItem)
				{
					nextItem = endItem+1;
				}
				else
				{
					throw strus::runtime_error( _TXT( "string as configuration value not terminated"));
				}
				while (*nextItem && (unsigned char)*nextItem <= 32) ++nextItem;
				if (*nextItem == ';')
				{
					++nextItem;
				}
				else if (*nextItem)
				{
					throw strus::runtime_error( _TXT( "extra token found after string value in configuration string"));
				}
			}
			else
			{
				// Value is a token:
				endItem = std::strchr( cc, ';');
				if (endItem)
				{
					nextItem = endItem+1;
				}
				else
				{
					nextItem = endItem = std::strchr( cc, '\0');
				}
				// Left trim of value:
				while (endItem > cc && (unsigned char)*(endItem-1) <= 32) --endItem;
			}
			if (utils::caseInsensitiveEquals( cfgkey, key))
			{
				res = std::string( cc, endItem - cc);
				config = std::string( config.c_str(), start) + nextItem;
				return true;
			}
			else
			{
				cc = nextItem;
			}
		}
		return false;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error extracting string from for key '%s' configuration string: %s"), key, *errorhnd, false);
}

static bool yesNoFromString( const char* cfgname, const std::string& str)
{
	std::string lostr = utils::tolower( str);
	if (lostr == "y") return true;
	if (lostr == "n") return false;
	if (lostr == "t") return true;
	if (lostr == "f") return false;
	if (lostr == "yes") return true;
	if (lostr == "no") return false;
	if (lostr == "true") return true;
	if (lostr == "false") return false;
	if (lostr == "1") return true;
	if (lostr == "0") return false;
	throw strus::runtime_error( _TXT( "value for configuration option '%s' is not a boolean (yes/no or true/false)"), cfgname);
}

DLL_PUBLIC bool strus::extractBooleanFromConfigString( bool& val, std::string& config, const char* key, ErrorBufferInterface* errorhnd)
{
	try
	{
		std::string cfgval;
		if (extractStringFromConfigString( cfgval, config, key, errorhnd))
		{
			val = yesNoFromString( key, cfgval);
			return true;
		}
		else
		{
			return false;
		}
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT( "error extracting boolean for key '%s' from configuration string: %s"), key, *errorhnd, false);
}

static unsigned int unsignedFromString( const std::string& numstr)
{
	unsigned int rt = 0;
	char const* cc = numstr.c_str();
	for (;*cc; ++cc)
	{
		if (*cc >= '0' && *cc <= '9')
		{
			rt = (rt * 10) + (*cc - '0');
		}
		else if (*cc == 'K' || *cc == 'k')
		{
			rt = rt * 1024;
			++cc;
			break;
		}
		else if (*cc == 'M' || *cc == 'm')
		{
			rt = rt * 1024 * 1024;
			++cc;
			break;
		}
		else if (*cc == 'G' || *cc == 'g')
		{
			rt = rt * 1024 * 1024 * 1024;
			++cc;
			break;
		}
		else
		{
			break;
		}
	}
	if (*cc)
	{
		throw strus::runtime_error( _TXT( "not a number (with optional 'K' or 'M' or 'G' suffix): '%s' (%s)"), numstr.c_str(), cc);
	}
	return rt;
}

static double doubleFromString( const std::string& numstr)
{
	double rt = 0.0;
	double frac = 0.0;
	bool got_dot = false;
	bool sign = false;
	char const* cc = numstr.c_str();
	if (*cc == '-')
	{
		sign = true;
		++cc;
	}
	for (;*cc; ++cc)
	{
		if (*cc >= '0' && *cc <= '9')
		{
			if (got_dot)
			{
				rt += (double)(unsigned int)(*cc - '0') * frac;
				frac /= 10;
			}
			else
			{
				rt = (rt * 10) + (*cc - '0');
			}
		}
		else if (*cc == '.')
		{
			if (got_dot) throw strus::runtime_error( _TXT("expected floating point number: %s"), numstr.c_str());
			got_dot = true;
			frac = 0.1;
		}
		else
		{
			break;
		}
	}
	if (sign) rt = -rt;
	if (*cc)
	{
		throw strus::runtime_error( _TXT( "not a number (with optional 'K' or 'M' or 'G' suffix) for configuration option: '%s'"), numstr.c_str());
	}
	return rt;
}

DLL_PUBLIC bool strus::extractUIntFromConfigString( unsigned int& val, std::string& config, const char* key, ErrorBufferInterface* errorhnd)
{
	try
	{
		std::string cfgval;
		if (extractStringFromConfigString( cfgval, config, key, errorhnd))
		{
			val = unsignedFromString( cfgval);
			return true;
		}
		else
		{
			return false;
		}
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error extracting unsigned integer for key '%s' from configuration string: %s"), key, *errorhnd, false);
}


DLL_PUBLIC bool strus::extractFloatFromConfigString( double& val, std::string& config, const char* key, ErrorBufferInterface* errorhnd)
{
	try
	{
		std::string cfgval;
		if (extractStringFromConfigString( cfgval, config, key, errorhnd))
		{
			val = doubleFromString( cfgval);
			return true;
		}
		else
		{
			return false;
		}
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error extracting floating point value for key '%s' from configuration string: %s"), key, *errorhnd, false);
}


DLL_PUBLIC void strus::removeKeysFromConfigString( std::string& config, const char** keys, ErrorBufferInterface* errorhnd)
{
	try
	{
		std::string val;
		for (std::size_t ii=0; keys[ii]; ++ii)
		{
			extractStringFromConfigString( val, config, keys[ii], errorhnd);
		}
	}
	CATCH_ERROR_MAP( _TXT("error removing keys from configuration string: %s"), *errorhnd);
}


