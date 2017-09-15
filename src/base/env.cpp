/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Accessing environment variables
#include "strus/base/env.hpp"
#include "strus/base/dll_tags.hpp"
#include "fileio_sys.hpp"
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
using namespace strus;


DLL_PUBLIC int strus::getenv( const std::string& var, std::string& output)
{
	const char* ev = ::getenv( var.c_str());
	try
	{
		if (ev)
		{
			output.clear();
			output.append( ev);
		}
		return 0;
	}
	catch (const std::bad_alloc&)
	{
		return 12/*ENOMEM*/;
	}
}

DLL_PUBLIC int strus::getenv_list( const std::string& var, char sep, std::vector<std::string>& output)
{
	const char* ev = ::getenv( var.c_str());
	try
	{
		if (ev)
		{
			char const* si = ev;
			const char* se = std::strchr( ev, STRUS_FILEIO_PATHSEP);
			for (; se; si = se+1, se = std::strchr( si, STRUS_FILEIO_PATHSEP))
			{
				if (se > si)
				{
					output.push_back( std::string( si, se-si));
				}
			}
			if (*si)
			{
				output.push_back( std::string( si));
			}
		}
		return 0;
	}
	catch (const std::bad_alloc&)
	{
		return 12/*ENOMEM*/;
	}
}

DLL_PUBLIC char strus::separatorPathList()
{
	return STRUS_FILEIO_PATHSEP;
}


