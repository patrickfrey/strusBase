/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Helper classes functions and templates for introspection
/// \file introspection.cpp
#include "strus/base/introspection.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/local_ptr.hpp"
#include "private/internationalization.hpp"
#include <iostream>
#include <sstream>

using namespace strus;

DLL_PUBLIC std::vector<std::string> strus::getIntrospectionElementList( const char** ar, class ErrorBufferInterface* errorhnd)
{
	try
	{
		std::vector<std::string> rt;
		char const** ai = ar;
		if (ai) for (; *ai; ++ai) rt.push_back( *ai);
		return rt;
	}
	catch (const std::bad_alloc&)
	{
		errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory building an introspection element list"));
		return std::vector<std::string>();
	}
}

DLL_PUBLIC char const* strus::introspectionBadAllocError()
{
	return _TXT("out of memory in introspection");
}

static void introspection_print( std::ostream& out, const std::string& indentstr, const IntrospectionInterface* introspection, int depth)
{
	if (depth == 0) return;
	std::string value( introspection->value());
	if (!value.empty())
	{
		out << indentstr << value << "\n";
	}
	std::vector<std::string> names( introspection->list());
	if (!names.empty())
	{
		std::string next_indentstr = indentstr + "  ";
		std::vector<std::string>::const_iterator ni = names.begin(), ne = names.end();
		for (; ni != ne; ++ni)
		{
			out << "\n" << indentstr << *ni << "\n";
			strus::local_ptr<IntrospectionInterface> next( introspection->open( *ni));
			if (next.get())
			{
				introspection_print( out, next_indentstr, next.get(), depth-1);
			}
		}
	}
}

DLL_PUBLIC std::string strus::introspection_tostring( const IntrospectionInterface* introspection, int maxdepth)
{
	try
	{
		std::ostringstream out;
		introspection_print( out, "", introspection, maxdepth);
		return out.str();
	}
	catch (const std::bad_alloc&)
	{
		return std::string();
	}
}

