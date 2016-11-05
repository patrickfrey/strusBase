/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/inputStream.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>
#include <cstring>
#include <cerrno>

using namespace strus;

DLL_PUBLIC InputStream::InputStream( const std::string& docpath)
	:m_fh(0),m_docpath(),m_bufferidx(0)
{
	try
	{
		m_docpath = docpath;
		if (docpath == "-")
		{
			m_fh = stdin;
		}
		else
		{
			m_fh = ::fopen( docpath.c_str(), "rb");
			if (!m_fh)
			{
				m_errno = errno;
			}
		}
	}
	catch (const std::bad_alloc&)
	{
		m_errno = ENOMEM;
	}
}

DLL_PUBLIC InputStream::~InputStream()
{
	if (m_fh && m_fh != stdin)
	{
		::fclose( m_fh);
	}
}

DLL_PUBLIC std::size_t InputStream::read( char* buf, std::size_t bufsize)
{
	try
	{
		m_errno = 0;
		if (!m_fh)
		{
			m_errno = EINVAL;
			return 0;
		}
		if (m_bufferidx > bufsize * 2)
		{
			m_buffer = std::string( m_buffer.c_str() + m_bufferidx, m_buffer.size() - m_bufferidx);
			m_bufferidx = 0;
		}
		if (!bufsize) return 0;
		unsigned int idx = 0;
		if (m_bufferidx < m_buffer.size())
		{
			std::size_t restsize = m_buffer.size() - m_bufferidx;
			if (restsize >= bufsize)
			{
				std::memcpy( buf, m_buffer.c_str()+m_bufferidx, bufsize);
				m_bufferidx += bufsize;
				return bufsize;
			}
			else
			{
				std::memcpy( buf, m_buffer.c_str()+m_bufferidx, restsize);
				m_bufferidx += restsize;
				idx = restsize;
			}
			if (m_bufferidx == m_buffer.size())
			{
				m_buffer.clear();
				m_bufferidx = 0;
			}
		}
		std::size_t rt = ::fread( buf + idx, 1, bufsize - idx, m_fh);
		if (!rt)
		{
			if (!feof( m_fh))
			{
				m_errno = ::ferror( m_fh);
				return 0;
			}
		}
		return idx + rt;
	}
	catch (const std::bad_alloc&)
	{
		m_errno = ENOMEM;
		return 0;
	}
}

DLL_PUBLIC std::size_t InputStream::readAhead( char* buf, std::size_t bufsize)
{
	try
	{
		if (!m_fh)
		{
			m_errno = EINVAL;
			return 0;
		}
		if (m_bufferidx > bufsize * 2)
		{
			m_buffer = std::string( m_buffer.c_str() + m_bufferidx, m_buffer.size() - m_bufferidx);
			m_bufferidx = 0;
		}
		if (!bufsize) return 0;
		std::size_t restsize = m_buffer.size() - m_bufferidx;
		if (restsize >= bufsize)
		{
			std::memcpy( buf, m_buffer.c_str() + m_bufferidx, bufsize);
			return bufsize;
		}
		else
		{
			std::size_t rt = ::fread( buf, 1, bufsize - restsize, m_fh);
			if (!rt)
			{
				if (!feof( m_fh))
				{
					m_errno = ::ferror( m_fh);
					return 0;
				}
			}
			if (restsize)
			{
				m_buffer.append( buf, rt);
				std::memcpy( buf, m_buffer.c_str() + m_bufferidx, restsize + rt);
				return restsize + rt;
			}
			else
			{
				m_bufferidx = 0;
				m_buffer = std::string( buf, rt);
				std::memcpy( buf, m_buffer.c_str(), rt);
				return rt;
			}
		}
	}
	catch (const std::bad_alloc&)
	{
		m_errno = ENOMEM;
		return 0;
	}
}

DLL_PUBLIC const char* InputStream::readLine( char* buf, std::size_t bufsize, bool failOnNoLine)
{
	try
	{
		m_errno = 0;
		if (!bufsize) return 0;
		char const* eolptr = ::strchr( m_buffer.c_str() + m_bufferidx, '\n');
		if (!eolptr)
		{
			(void)readAhead( buf, bufsize);
			eolptr = ::strchr( m_buffer.c_str() + m_bufferidx, '\n');
		}
		const char* ptr = m_buffer.c_str() + m_bufferidx;
		if (eolptr)
		{
			std::size_t len = eolptr - ptr;
			if (len >= bufsize)
			{
				if (failOnNoLine)
				{
					m_errno = EINVAL;
					return 0;
				}
				else
				{
					std::memcpy( buf, ptr, bufsize-1);
					buf[ bufsize-1] = '\0';
					m_bufferidx += bufsize-1;
				}
			}
			else
			{
				std::memcpy( buf, ptr, len);
				buf[ len] = '\0';
				m_bufferidx += len + 1/*\n*/;
			}
		}
		else if (failOnNoLine)
		{
			m_errno = EINVAL;
			return 0;
		}
		else
		{
			std::size_t restsize = m_buffer.size() - m_bufferidx;
			if (restsize == 0) return 0;
			std::size_t nn = (restsize >= bufsize)?(bufsize-1):restsize;
			std::memcpy( buf, ptr, nn);
			buf[ nn] = '\0';
			m_bufferidx += nn;
		}
		return buf;
	}
	catch (const std::bad_alloc&)
	{
		m_errno = ENOMEM;
		return 0;
	}
}

DLL_PUBLIC unsigned int InputStream::error() const
{
	return m_errno;
}






