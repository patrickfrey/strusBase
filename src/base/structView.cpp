/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Datastructure for introspection of structures
/// \file structView.cpp
#include "strus/structView.hpp"
#include "strus/base/dll_tags.hpp"

using namespace strus;

DLL_PUBLIC bool StructView::appendToString( std::string& buf) const
{
	switch (m_type)
	{
		case Null: return true;
		case String:
		{
			buf.push_back('\"');
			buf.append( m_string);
			buf.push_back('\"');
			return true;
		}
		case Numeric:
		{
			buf.append( m_numeric.tostring( FloatPrecisionToString).c_str());
			return true;
		}
		case Structure:
		{
			buf.push_back('{');
			std::vector<StructView>::const_iterator ai = m_ar.begin(), ae = m_ar.end();
			int nn = 0;
			for (; ai != ae; ++ai,++nn)
			{
				if (nn) buf.push_back(',');
				ai->appendToString( buf);
			}
			std::map<std::string,StructView>::const_iterator di =  m_dict.begin(), de =  m_dict.end();
			for (; di != de; ++di,++nn)
			{
				if (nn) buf.push_back(',');
				buf.append( di->first);
				buf.push_back(':');
				di->second.appendToString( buf);
			}
			buf.push_back('}');
			return true;
		}
	}
	return false;
}


