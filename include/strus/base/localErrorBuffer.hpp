/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Error buffer implementation for a context transforming the error message into an exceptions at the end of its scope
/// \file localErrorBuffer.hpp
#ifndef _STRUS_LOCAL_ERROR_BUFFER_HPP_INCLUDED
#define _STRUS_LOCAL_ERROR_BUFFER_HPP_INCLUDED
#include "strus/errorCodes.hpp"
#include "strus/errorBufferInterface.hpp"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <stdexcept>

/// \brief strus toplevel namespace
namespace strus
{

/// \class LocalErrorBuffer
/// \brief Error buffer implementation for a context transforming the error message into an exceptions at the end of its scope
class LocalErrorBuffer :public ErrorBufferInterface
{
public:
	LocalErrorBuffer()
		:m_errorCode(-1)
	{
		m_buf[0] = 0;
	}

	virtual ~LocalErrorBuffer(){}

	virtual void setLogFile( FILE* )
	{
		throw std::logic_error( "not implemented");
	}

	virtual bool setMaxNofThreads( unsigned int)
	{
		throw std::logic_error( "not implemented");
	}

	virtual void report( int errorcode, const char* format, ...)
	{
		va_list ap;
		va_start( ap, format);
		std::size_t hdrlen = 0;
		if (errorcode)
		{
			hdrlen = std::snprintf( m_buf, sizeof(m_buf), "##%d ", errorcode);
		}
		std::vsnprintf( m_buf + hdrlen, sizeof(m_buf) - hdrlen, format, ap);
		m_buf[ sizeof(m_buf)-1] = 0;
		va_end( ap);
		m_errorCode = errorcode;
	}

	virtual void explain( const char* format)
	{
		char tmpbuf[ BufferSize];
		std::size_t len = std::snprintf( tmpbuf, sizeof(tmpbuf), format, m_buf);
		if (len >= sizeof(tmpbuf))
		{
			len = sizeof(tmpbuf)-1;
			tmpbuf[ len] = 0;
		}
		std::memcpy( m_buf, tmpbuf, len);
		m_buf[ len] = 0;
	}

	virtual const char* fetchError()
	{
		if (m_errorCode >= 0)
		{
			m_errorCode = -1;
			return m_buf;
		}
		return 0;
	}

	virtual bool hasError() const
	{
		return m_errorCode >= 0;
	}

	virtual void allocContext()
	{
		throw std::logic_error( "not implemented");
	}

	virtual void releaseContext()
	{
		throw std::logic_error( "not implemented");
	}

	int errorCode() const
	{
		return m_errorCode;
	}

private:
	enum {BufferSize=2048};
	mutable char m_buf[ BufferSize];
	int m_errorCode;
};

}//namespace
#endif

