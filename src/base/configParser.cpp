/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/configParser.hpp"
#include "strus/base/numstring.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/string_conv.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include <map>
#include <limits>
#include <cstring>
#include <stdexcept>

using namespace strus;

static bool parseNextConfigItem( char const*& src, std::string& cfgkey, const char*& valuestart, std::size_t& valuesize)
{
	cfgkey.clear();

	char const* cc = src;
	while (*cc && ((unsigned char)*cc <= 32 || *cc == ';')) ++cc;
	//... skip spaces and delimiters
	if (!*cc) return false;

	while (((*cc|32) >= 'a' && (*cc|32) <= 'z') || *cc == '_' || (*cc >= '0' && *cc <= '9'))
	{
		cfgkey.push_back( *cc++);
	}
	if (cfgkey.empty())
	{
		throw strus::runtime_error( _TXT( "expected item identifier as start of a declaration in a config string ('%s' | '%s')"), cfgkey.c_str(), src);
	}
	if (*cc != '=')
	{
		throw strus::runtime_error( _TXT( "'=' expected after item identifier in a config string ('%s %s' | '%s')"), cfgkey.c_str(), cc, src);
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
			throw strus::runtime_error( "%s", _TXT( "string as configuration value not terminated"));
		}
		while (*nextItem && (unsigned char)*nextItem <= 32) ++nextItem;
		if (*nextItem == ';')
		{
			++nextItem;
		}
		else if (*nextItem)
		{
			throw strus::runtime_error( "%s", _TXT( "extra token found after string value in configuration string"));
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
	valuestart = cc;
	valuesize = endItem - cc;
	src = nextItem;
	return true;
}

typedef std::pair<std::string,std::string> ConfigItem;
typedef std::vector<ConfigItem> ConfigItemList;

DLL_PUBLIC ConfigItemList strus::getConfigStringItems( const std::string& config, ErrorBufferInterface* errorhnd)
{
	try
	{
		ConfigItemList rt;
		std::string cfgkey;
		const char* valuestart;
		std::size_t valuesize;
		StringConvError errcode = StringConvOk;

		char const* cc = config.c_str();
		while (parseNextConfigItem( cc, cfgkey, valuestart, valuesize))
		{
			rt.push_back( ConfigItem( strus::tolower(cfgkey, errcode), std::string( valuestart, valuesize)));
			if (errcode != StringConvOk) throw strus::stringconv_exception( errcode);
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error parsing configuration string items: %s"), *errorhnd, ConfigItemList());
}

DLL_PUBLIC bool strus::extractStringFromConfigString( std::string& res, std::string& config, const char* key, ErrorBufferInterface* errorhnd)
{
	try
	{
		std::string cfgkey;
		const char* valuestart;
		std::size_t valuesize;

		char const* cc = config.c_str();
		char const* lastptr = cc;
		while (parseNextConfigItem( cc, cfgkey, valuestart, valuesize))
		{
			if (strus::caseInsensitiveEquals( cfgkey, key))
			{
				res = std::string( valuestart, valuesize);
				config = std::string( config.c_str(), lastptr) + std::string(cc);
				return true;
			}
			lastptr = cc;
		}
		return false;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error extracting string from for key '%s' configuration string: %s"), key, *errorhnd, false);
}

static bool yesNoFromString( const char* cfgname, const std::string& str)
{
	StringConvError errcode = StringConvOk;
	std::string lostr = strus::tolower( str, errcode);
	if (errcode != StringConvOk) throw strus::stringconv_exception( errcode);
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

static bool checkError( NumParseError err, const char* type, ErrorBufferInterface* errorhnd)
{
	switch (err)
	{
		case NumParseOk:
			return true;
		case NumParseErrNoMem:
			errorhnd->report(_TXT("failed to extract %s from configuration string: %s"), type, _TXT("out of memory"));
			return false;
		case NumParseErrConversion:
			errorhnd->report(_TXT("failed to extract %s from configuration string: %s"), type, _TXT("conversion error"));
			return false;
		case NumParseErrOutOfRange:
			errorhnd->report(_TXT("failed to extract %s from configuration string: %s"), type, _TXT("value out of range"));
			return false;
	}
	errorhnd->report(_TXT("failed to extract %s from configuration string: %s"), type, _TXT("undefined error code"));
	return false;
}

DLL_PUBLIC bool strus::extractUIntFromConfigString( unsigned int& val, std::string& config, const char* key, ErrorBufferInterface* errorhnd)
{
	std::string cfgval;
	if (extractStringFromConfigString( cfgval, config, key, errorhnd))
	{
		NumParseError err = NumParseOk;
		val = uintFromString( cfgval, std::numeric_limits<unsigned int>::max(), err);
		return checkError( err, "UINT", errorhnd);
	}
	else
	{
		return false;
	}
}


DLL_PUBLIC bool strus::extractFloatFromConfigString( double& val, std::string& config, const char* key, ErrorBufferInterface* errorhnd)
{
	try
	{
		std::string cfgval;
		if (extractStringFromConfigString( cfgval, config, key, errorhnd))
		{
			NumParseError err = NumParseOk;
			val = doubleFromString( cfgval, err);
			return checkError( err, "FLOAT", errorhnd);
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

DLL_PUBLIC bool strus::addConfigStringItem( std::string& config, const std::string& key, const std::string& value, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!config.empty())
		{
			config.push_back(';');
		}
		enum ValueType {Identifier, String, SQString, DQString};
		ValueType valueType = Identifier;
		config.append( key);
		config.push_back( '=');
		std::string::const_iterator ci = value.begin(), ce = value.end();
		for (; ci != ce; ++ci)
		{
			if ((unsigned char)*ci < 32) throw strus::runtime_error( "%s", _TXT( "unsupported control character in configuration value"));
			if (*ci == '"')
			{
				if (valueType == DQString) throw strus::runtime_error( "%s", _TXT( "cannot add configuration value with to types of quotes"));
				valueType = SQString;
			}
			else if (*ci == '\'')
			{
				if (valueType == SQString) throw strus::runtime_error( "%s", _TXT( "cannot add configuration value with to types of quotes"));
				valueType = DQString;
			}
			else if (*ci == ';' || *ci == ' ')
			{
				if (valueType == Identifier) valueType = String;
			}
		}
		switch (valueType)
		{
			case Identifier:
				config.append( value);
				break;
			case String:
			case SQString:
				config.push_back( '\'');
				config.append( value);
				config.push_back( '\'');
				break;
			case DQString:
				config.push_back( '"');
				config.append( value);
				config.push_back( '"');
				break;
		}
		return true;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error adding value for key '%s' to configuration string: %s"), key.c_str(), *errorhnd, false);
}

