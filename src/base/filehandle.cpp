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
private:
	int pipfd[2];
	int pipfd_signal[2];
	fd_set readfds;
	int readfds_size;
	strus::mutex mutex_buffer;
	std::string buffer;
	strus::thread* thread;
	int ec;
	WriteBufferHandle::ErrorHandler errorHandler;
	void* errorHandlerCtx;
	FILE* streamHandle;
	enum State {StateInit=0, StateWait=1, StateData=2, StateStopped=3};
	AtomicCounter<int> state;

public:
	explicit Data( WriteBufferHandle::ErrorHandler errorHandler_, void* errorHandlerCtx_)
	{
		errorHandler = errorHandler_;
		errorHandlerCtx = errorHandlerCtx_;
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
		if (errorHandler) errorHandler( errorHandlerCtx, ec);
	}

	~Data()
	{
		closefd();
	}

	int writeFileHandle() const
	{
		return pipfd[ 1];
	}

	int error() const
	{
		int rt = ec;
		return (rt == EAGAIN || rt == EINTR) ? 0 : rt;
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

	void readData()
	{
		std::string data;
		for (;;)
		{
			char buf[ 4096];
			ssize_t nn = ::read( pipfd[0], buf, sizeof(buf));
			if (nn > 0)
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
				setErrno( errno);
				maskErrno( EINTR);
				if (ec || nn < (ssize_t)sizeof(buf)) break;
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
		if (ec)
		{
			if (errorHandler) errorHandler( errorHandlerCtx, ec);
		}
	}

	void run()
	{
		(void)state.test_and_set( StateInit, StateData);
		do
		{
			readData();

			if (state.test_and_set( StateData, StateWait))
			{
				waitReadAvailable();
				(void)state.test_and_set( StateWait, StateData);
			}
		} while (state.value() != StateStopped);

		readData();
		state.set( StateInit);
	}

	void start()
	{
		// ... can be called only once
		try
		{
			if (thread)
			{
				ec = EPERM;
			}
			else
			{
				thread = new strus::thread( &WriteBufferHandle::Data::run, this);
			}
		}
		catch (...)
		{
			ec = ENOMEM;
			thread = 0;
			if (errorHandler) errorHandler( errorHandlerCtx, ec);
		}
	}

	void stop()
	{
		if (state.value() == StateInit) return;
		flushBuffer();

		if (state.test_and_set( StateWait, StateStopped) || state.test_and_set( StateData, StateStopped))
		{
			signalReadEvent();
		}
		if (thread) thread->join();
		delete thread;
		thread = 0;
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
			if (errorHandler) errorHandler( errorHandlerCtx, ec);
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
				while (state.value() == StateData)
				{
					strus::usleep( 1L);
				}
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

private:
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
};

DLL_PUBLIC WriteBufferHandle::WriteBufferHandle( ErrorHandler errorHandler, void* errorHandlerCtx)
{
	try
	{
		m_impl = new Data( errorHandler, errorHandlerCtx);
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
	return m_impl ? m_impl->writeFileHandle() : 0;//...write end of pipe
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
	return m_impl ? m_impl->error() : ENOMEM;
}

DLL_PUBLIC void WriteBufferHandle::close()
{
	if (m_impl) m_impl->stop();
}

