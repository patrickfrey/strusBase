/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some functions to parse a configuration string (e.g. strus storage configuration)
#ifndef _STRUS_CONFIG_PARSER_HPP_INCLUDED
#define _STRUS_CONFIG_PARSER_HPP_INCLUDED
#include <string>
#include <vector>
#include <utility>

namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;

bool extractStringFromConfigString( std::string& val, std::string& config, const char* key, ErrorBufferInterface* errorhnd);
bool extractBooleanFromConfigString( bool& val, std::string& config, const char* key, ErrorBufferInterface* errorhnd);
bool extractUIntFromConfigString( unsigned int& val, std::string& config, const char* key, ErrorBufferInterface* errorhnd);
bool extractFloatFromConfigString( double& val, std::string& config, const char* key, ErrorBufferInterface* errorhnd);
void removeKeysFromConfigString( std::string& config, const char** keys, ErrorBufferInterface* errorhnd);

bool addConfigStringItem( std::string& config, const std::string& key, const std::string& value, ErrorBufferInterface* errorhnd);
std::vector<std::pair<std::string,std::string> > getConfigStringItems( const std::string& config, ErrorBufferInterface* errorhnd);
std::vector<std::pair<std::string,std::string> > getSubConfigStringItems( const std::string& configelem, ErrorBufferInterface* errorhnd);

}//namespace
#endif

