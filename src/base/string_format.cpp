/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
///\brief Get a string built by a format string
#include "strus/base/string_format.hpp"
#include "strus/base/dll_tags.hpp"
#include <cstdio>
#include <cstdlib>

using namespace strus;

DLL_PUBLIC std::string strus::string_format( const char* fmt, va_list ap)
{
	std::string rt;
	char msgbuf[ 4096];
	va_list apcopy;
	va_copy( apcopy, ap);
	int len = ::vsnprintf( msgbuf, sizeof(msgbuf), fmt, ap);
	if (len < (int)sizeof( msgbuf))
	{
		try
		{
			rt.append( msgbuf);
		}
		catch (const std::bad_alloc&)
		{
			return std::string();
		}
	}
	else
	{
		char* msgptr = (char*)std::malloc( len+1);
		if (msgptr)
		{
			::vsnprintf( msgptr, len+1, fmt, apcopy);
			try
			{
				rt.append( msgptr, len);
				std::free( msgptr);
			}
			catch (const std::bad_alloc&)
			{
				std::free( msgptr);
				return std::string();
			}
		}
	}
	return rt;
}

DLL_PUBLIC std::string strus::string_format( const char* fmt, ...)
{
	std::string rt;
	va_list ap;
	va_start( ap, fmt);
	rt = string_format( fmt, ap);
	va_end( ap);
	return rt;
}


