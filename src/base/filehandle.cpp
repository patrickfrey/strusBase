/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Basic portable file/socket handle and related system functions
/// \file "filehandle.cpp"
#include "strus/base/filehandle.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/thread.hpp"
#include <unistd.h>
#include <fcntl.h>

using namespace strus;

struct WriteBufferHandle::Data
{
	int pipfd[2];
	strus::mutex mutex;
	std::string buffer;
	strus::thread* thread;
	int ec;
	bool stopped;

	Data()
	{
		pipfd[0] = 0;
		pipfd[1] = 0;
		if (::pipe(pipfd) == -1) throw std::bad_alloc();
		ec = 0;
		stopped = false;
	}

	~Data()
	{
		::close( pipfd[0]);
		::close( pipfd[1]);
	}

	bool appendData( const char* buf, std::size_t size)
	{
		try
		{
			strus::unique_lock lock( mutex);
			buffer.append( buf, size);
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	std::string fetchContent()
	{
		try
		{
			std::string rt;
			strus::unique_lock lock( mutex);
			std::swap( buffer, rt);
			return rt;
		}
		catch (...)
		{
			return std::string();
		}
	}

	void wait()
	{
		char buf[ 4096];
		for (;;)
		{
			ssize_t nn = ::read( pipfd[0], buf, sizeof(buf));
			if (nn > 0)
			{
				bool eof = false;
				const char* eofp = (const char*)std::memchr( buf, '\0', nn);
				if (eofp)
				{
					nn = eofp - buf;
					eof = true;
				}
				if (!appendData( buf, nn))
				{
					ec = 12/*ENOMEM*/;
					return;
				}
				if (eof) return;
			}
			else
			{
				ec = errno;
				if (ec && ec != 4/*EINTR*/) continue;
				return;
			}
		}
	}

	void start()
	{
		// ... can be called only once
		try
		{
			thread = new strus::thread( &WriteBufferHandle::Data::wait, this);
		}
		catch (...)
		{
			ec = 12/*ENOMEM*/;
			thread = 0;
		}
	}

	void stop()
	{
		if (stopped) return;

		while (1 != ::write( pipfd[1], "", 1))
		{
			ec = errno;
			if (ec != 4/*EINTR*/) return;
		}
		if (thread) thread->join();
		stopped = true;
	}
};

DLL_PUBLIC WriteBufferHandle::WriteBufferHandle()
{
	try
	{
		m_impl = new Data();
		m_impl->start();
	}
	catch (...)
	{
		m_impl = 0;
	}
}

DLL_PUBLIC WriteBufferHandle::~WriteBufferHandle()
{
	if (m_impl)
	{
		m_impl->stop();
		delete m_impl;
	}
}

DLL_PUBLIC FileHandle WriteBufferHandle::fileHandle() const
{
	return m_impl ? m_impl->pipfd[1] : 0;//...write end of pipe
}

DLL_PUBLIC std::string WriteBufferHandle::fetchContent()
{
	return m_impl ? m_impl->fetchContent() : std::string();
}

DLL_PUBLIC int WriteBufferHandle::error() const
{
	return m_impl ? m_impl->ec : 12/*ENOMEM*/;
}

DLL_PUBLIC void WriteBufferHandle::done()
{
	if (m_impl) m_impl->stop();
}

