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

static inline bool isSpace( char ch)
{
	return (ch == '\n' || ch == '\t' || ch == '\r' || ch == ' ');
}

static inline char const* skipSpaces( char const* cc)
{
	while (isSpace(*cc)) ++cc;
	return cc;
}

static bool parseIdentifier( char const*& src, std::string& ident)
{
	ident.clear();
	char const* cc = src;

	while (isSpace(*cc) || *cc == ';') ++cc;
	if (!*cc) return false;

	while (((*cc|32) >= 'a' && (*cc|32) <= 'z') || *cc == '_' || (*cc >= '0' && *cc <= '9'))
	{
		ident.push_back( *cc++);
	}
	if (ident.empty()) return false;
	src = cc;
	return true;
}

struct Token
{
	char const* start;
	char const* end;

	Token() :start(0),end(0) {}

	std::size_t size() const	{return end-start;}
	std::string str() const		{return std::string( start, end-start);}
};

static bool parseToken( char const*& src, Token& token, char delim)
{
	char const* nextItem;
	char const* cc = skipSpaces(src);
	if (*cc == '"' || *cc == '\'')
	{
		// ... value is a string (without any escaping of characters with backslash):
		char eb = *cc++;
		token.start = cc;
		for (token.end=cc; *token.end != '\0' && *token.end != eb; ++token.end){}
		if (*token.end)
		{
			nextItem = token.end+1;
		}
		else
		{
			throw std::runtime_error( _TXT( "string in configuration not terminated"));
		}
		nextItem = skipSpaces( nextItem);
		if (*nextItem == delim)
		{
			++nextItem;
		}
		else if (*nextItem)
		{
			throw std::runtime_error( _TXT( "extra token found after string value in configuration string"));
		}
	}
	else
	{
		// ... value is a token:
		token.start = cc;
		token.end = std::strchr( cc, delim);
		if (token.end)
		{
			nextItem = token.end+1;
		}
		else
		{
			nextItem = token.end = std::strchr( cc, '\0');
		}
		// Left trim of value:
		while (token.end > cc && isSpace(*(token.end-1))) --token.end;
	}
	src = nextItem;
	return true;
}

static bool parseNextConfigItem( char const*& src, std::string& key, Token& token, char separator)
{
	src = skipSpaces(src);
	if (!*src) return false;
	if (!parseIdentifier( src, key))
	{
		throw strus::runtime_error( _TXT( "expected item identifier as start of a declaration in a config string ('%s')"), src);
	}
	src = skipSpaces( src);
	if (*src != '=')
	{
		throw strus::runtime_error( _TXT( "'=' expected after item identifier in a config string ('%s') at '%s')"), key.c_str(), src);
	}
	src = skipSpaces( src+1);
	return parseToken( src, token, separator);
}

static bool parseNextSubConfigItem( char const*& src, std::string& key, Token& token)
{
	src = skipSpaces(src);
	if (!*src) return false;

	if (!parseIdentifier( src, key))
	{
		throw strus::runtime_error( _TXT( "expected item identifier as start of a declaration in a config string ('%s')"), src);
	}
	src = skipSpaces( src);
	return parseToken( src, token, ',');
}

typedef std::pair<std::string,std::string> ConfigItem;
typedef std::vector<ConfigItem> ConfigItemList;

DLL_PUBLIC ConfigItemList strus::getConfigStringItems( const std::string& config, ErrorBufferInterface* errorhnd)
{
	try
	{
		ConfigItemList rt;
		std::string cfgkey;
		Token token;
		StringConvError errcode = StringConvOk;

		char const* cc = config.c_str();
		while (parseNextConfigItem( cc, cfgkey, token, ';'))
		{
			rt.push_back( ConfigItem( strus::tolower( cfgkey, errcode), token.str()));
			if (errcode != StringConvOk) throw strus::stringconv_exception( errcode);
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error parsing configuration string items: %s"), *errorhnd, ConfigItemList());
}

DLL_PUBLIC ConfigItemList strus::getAssignmentListItems( const std::string& config, ErrorBufferInterface* errorhnd)
{
	try
	{
		ConfigItemList rt;
		std::string cfgkey;
		Token token;
		StringConvError errcode = StringConvOk;

		char const* cc = config.c_str();
		while (parseNextConfigItem( cc, cfgkey, token, ','))
		{
			rt.push_back( ConfigItem( strus::tolower( cfgkey, errcode), token.str()));
			if (errcode != StringConvOk) throw strus::stringconv_exception( errcode);
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error parsing assignment list items: %s"), *errorhnd, ConfigItemList());
}

DLL_PUBLIC std::vector<std::pair<std::string,std::string> > strus::getSubConfigStringItems( const std::string& configelem, ErrorBufferInterface* errorhnd)
{
	try
	{
		ConfigItemList rt;
		std::string cfgkey;
		Token token;
		StringConvError errcode = StringConvOk;

		char const* cc = configelem.c_str();
		while (parseNextSubConfigItem( cc, cfgkey, token))
		{
			rt.push_back( ConfigItem( strus::tolower( cfgkey, errcode), token.str()));
			if (errcode != StringConvOk) throw strus::stringconv_exception( errcode);
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error parsing sub configuration string items: %s"), *errorhnd, ConfigItemList());
}

DLL_PUBLIC bool strus::extractStringFromConfigString( std::string& res, std::string& config, const char* key, ErrorBufferInterface* errorhnd)
{
	try
	{
		std::string cfgkey;
		Token token;

		char const* cc = config.c_str();
		char const* lastptr = cc;

		while (parseNextConfigItem( cc, cfgkey, token, ';'))
		{
			if (strus::caseInsensitiveEquals( cfgkey, key))
			{
				res = token.str();
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
			errorhnd->report( ErrorCodeOutOfMem, _TXT("failed to extract %s from configuration string: %s"), type, _TXT("out of memory"));
			return false;
		case NumParseErrConversion:
			errorhnd->report( ErrorCodeSyntax, _TXT("failed to extract %s from configuration string: %s"), type, _TXT("conversion error"));
			return false;
		case NumParseErrOutOfRange:
			errorhnd->report( ErrorCodeValueOutOfRange, _TXT("failed to extract %s from configuration string: %s"), type, _TXT("value out of range"));
			return false;
		case NumParseErrInvalidArg:
			errorhnd->report( ErrorCodeInvalidArgument, _TXT("failed to extract %s from configuration string: %s"), type, _TXT("invalid argument"));
			return false;
	}
	errorhnd->report( ErrorCodeUnknown, _TXT("failed to extract %s from configuration string: %s"), type, _TXT("undefined error code"));
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
			if ((unsigned char)*ci < 32) throw std::runtime_error( _TXT( "unsupported control character in configuration value"));
			if (*ci == '"')
			{
				if (valueType == DQString) throw std::runtime_error( _TXT( "cannot add configuration value with to types of quotes"));
				valueType = SQString;
			}
			else if (*ci == '\'')
			{
				if (valueType == SQString) throw std::runtime_error( _TXT( "cannot add configuration value with to types of quotes"));
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

