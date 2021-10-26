/*
 * Copyright (c) 2021 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Reduced implementation of a string view type (to be replaced by the C++ std::string_view type when going to C++17)
#ifndef _STRUS_STRING_VIEW_HPP_INCLUDED
#define _STRUS_STRING_VIEW_HPP_INCLUDED
#include <string>
#include <cstring>

namespace strus {

class string_view
{
public:
	string_view( const char* data_)			:m_data(data_),m_size(std::strlen(data_)){}
	string_view( const char* data_, size_t size_)	:m_data(data_),m_size(size_){}
	string_view( const std::string& str)		:m_data(str.c_str()),m_size(str.size()){}
	string_view( const string_view& o)		:m_data(o.m_data),m_size(o.m_size){}

	bool operator < (const string_view& o) const noexcept
	{
		if (m_size < o.m_size)
		{
			int cmpres = std::memcmp( m_data, o.m_data, m_size);
			return cmpres <= 0;
		}
		else
		{
			int cmpres = std::memcmp( m_data, o.m_data, o.m_size);
			return cmpres < 0;
		}
	}
	bool operator == (const string_view& o) const noexcept
	{
		return m_size == o.m_size && 0==std::memcmp( m_data, o.m_data, o.m_size);
	}

private:
	const char* m_data;
	size_t m_size;
};

}//namespace
#endif
