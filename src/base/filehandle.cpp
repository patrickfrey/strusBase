/*
 * Copyright (c) 2019 Patrick P. Frey
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
#include "strus/base/atomic.hpp"
#include "strus/base/sleep.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>

using namespace strus;


struct WriteBufferHandle::Data
{
	int pipfd[2];
	int pipfd_signal[2];
	fd_set readfds;
	int readfds_size;
	strus::mutex mutex_buffer;
	std::string buffer;
	strus::thread* thread;
	int ec;
	FILE* streamHandle;
	enum State {StateInit=0, StateWait=1, StateData=2, StateStopped=3};
	AtomicCounter<int> state;

	Data()
	{
		int flags;

		pipfd[0] = 0;
		pipfd[1] = 0;
		if (::pipe(pipfd) == -1) goto ERROR;
		pipfd_signal[0] = 0;
		pipfd_signal[1] = 0;
		if (::pipe(pipfd_signal) == -1) goto ERROR;
		thread = 0;
		ec = 0;
		streamHandle = NULL;

		flags = ::fcntl( pipfd[0], F_GETFL, 0);
		::fcntl( pipfd[0], F_SETFL, flags | O_NONBLOCK);
		flags = ::fcntl( pipfd_signal[0], F_GETFL, 0);
		::fcntl( pipfd_signal[0], F_SETFL, flags | O_NONBLOCK);

		FD_ZERO( &readfds);
		FD_SET( pipfd[0], &readfds);
		FD_SET( pipfd_signal[0], &readfds);
		readfds_size = pipfd[0] > pipfd_signal[0] ? (pipfd[0]+1) : (pipfd_signal[0]+1);
		state.set( StateInit);
		return;
	ERROR:
		setErrno( errno);
		closefd();
	}

	bool setErrno( int ec_)
	{
		ec = ec_;
		return ec != 0;
	}

	bool maskErrno( int ec_)
	{
		if (ec_ == ec)
		{
			ec = 0;
			return true;
		}
		return false;
	}

	void closefd()
	{
		if (pipfd[0])
		{
			::close( pipfd[0]);
			pipfd[0] = 0;
		}
		if (streamHandle)
		{
			::fclose( streamHandle);
			pipfd[1] = 0;
		}
		else if (pipfd[1])
		{
			::close( pipfd[1]);
			pipfd[1] = 0;
		}
		if (pipfd_signal[0])
		{
			::close( pipfd_signal[0]);
			pipfd_signal[0] = 0;
		}
		if (pipfd_signal[1])
		{
			::close( pipfd_signal[1]);
			pipfd_signal[1] = 0;
		}
	}

	~Data()
	{
		closefd();
	}

	std::string fetchContent()
	{
		try
		{
			flushBuffer();

			std::string rt;
			strus::unique_lock lock( mutex_buffer);
			std::swap( buffer, rt);
			return rt;
		}
		catch (...)
		{
			return std::string();
		}
	}

	void run()
	{
		(void)state.test_and_set( StateInit, StateWait);
		while (state.value() != StateStopped)
		{
			(void)state.test_and_set( StateData, StateWait);
			waitReadAvailable();
			(void)state.test_and_set( StateWait, StateData);
			std::string data;
			for (;;)
			{
				char buf[ 4096];
				ssize_t nn = ::read( pipfd[0], buf, sizeof(buf));
				if (nn == 0)
				{
					break;
				}
				else if (nn > 0)
				{
					try
					{
						data.append( buf, nn);
					}
					catch (...)
					{
						ec = ENOMEM;
						break;
					}
				}
				else
				{
					if (setErrno( errno) && !maskErrno( EINTR)) break;
					if (nn < (ssize_t)sizeof(buf))
					{
						break;
					}
				}
			}
			maskErrno( EAGAIN);
			if (!data.empty())
			{
				strus::unique_lock lock( mutex_buffer);
				if (buffer.empty())
				{
					std::swap( buffer, data);
				}
				else
				{
					buffer.append( data);
				}
			}
		}
		state.set( StateInit);
	}

	void start()
	{
		// ... can be called only once
		try
		{
			thread = new strus::thread( &WriteBufferHandle::Data::run, this);
		}
		catch (...)
		{
			ec = ENOMEM;
			thread = 0;
		}
	}

	void stop()
	{
		if (state.value() == StateInit) return;
		if (state.test_and_set( StateWait, StateStopped) || state.test_and_set( StateData, StateStopped))
		{
			flushBuffer();
		}
		if (thread) thread->join();
	}

	void signalReadEvent()
	{
		ssize_t nn;
	AGAIN:
		char ch = ' ';
		nn = ::write( pipfd_signal[1], &ch, 1);
		if (nn <= 0)
		{
			setErrno( errno);
			if (maskErrno( EINTR) || maskErrno( EAGAIN)) goto AGAIN;
		}
	}

	bool waitReadAvailable()
	{
		struct timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		FD_SET( pipfd[0], &readfds);
		FD_SET( pipfd_signal[0], &readfds);

		if (0>::select( readfds_size, &readfds, NULL, NULL, &timeout))
		{
			setErrno( errno);
			maskErrno( EINTR);
			maskErrno( EAGAIN);
		}
		if (FD_ISSET( pipfd_signal[0], &readfds))
		{
			char buf[ 128];
			if (0>::read( pipfd_signal[0], buf, sizeof(buf)))
			{
				setErrno( errno);
				maskErrno( EINTR);
				maskErrno( EAGAIN);
			}
		}
		return FD_ISSET( pipfd[0], &readfds);
	}

	void flushBuffer()
	{
		if (streamHandle)
		{
		AGAIN:
			if (0>::fflush( streamHandle))
			{
				setErrno( errno);
				if (maskErrno( EINTR) || maskErrno( EAGAIN)) goto AGAIN;
			}
		}
		state.test_and_set( StateWait, StateData);
		signalReadEvent();
		while (state.value() == StateData)
		{
			strus::usleep( 1L);
		}
	}

	FILE* getCStreamHandle()
	{
		if (!streamHandle)
		{
			streamHandle = ::fdopen( pipfd[1], "a");
		}
		return streamHandle;
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

DLL_PUBLIC FILE* WriteBufferHandle::getCStreamHandle()
{
	return m_impl ? m_impl->getCStreamHandle() : NULL;
}
	
DLL_PUBLIC std::string WriteBufferHandle::fetchContent()
{
	return m_impl ? m_impl->fetchContent() : std::string();
}

DLL_PUBLIC int WriteBufferHandle::error() const
{
	return m_impl ? m_impl->ec : ENOMEM;
}

DLL_PUBLIC void WriteBufferHandle::done()
{
	if (m_impl) m_impl->stop();
}

