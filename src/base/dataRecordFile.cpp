/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Basic portable file type for collections of equal sized records with random access
#include "strus/base/dataRecordFile.hpp"
#include "strus/base/dll_tags.hpp"
#include "fileio_sys.hpp"
#include <cstdio>
#include <cerrno>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <limits>

using namespace strus;

#ifdef _MSC_VER
class DataRecordFile {};
#else
DLL_PUBLIC DataRecordFile::DataRecordFile()
	:m_fh(0),m_mode(NoAccess),m_errno(0),m_recordsize(0),m_headersize(0),m_recordindex(0),m_recbuf(0)
{}

DLL_PUBLIC DataRecordFile::~DataRecordFile()
{
	(void)close();
}

DLL_PUBLIC bool DataRecordFile::open( const std::string& filename, const Mode& mode_, unsigned int recordsize_, unsigned int headersize_, const void* headerstruct)
{
	if (m_fh && !close()) return false;
	if (!recordsize_)
	{
		m_errno = EINVAL;
		return false;
	}
	switch (mode_)
	{
		case NoAccess:
			m_fh = 0;
			m_errno = 0;
			break;
		case SharedRead:
			m_recbuf = std::malloc( recordsize_);
			if (!m_recbuf)
			{
				m_errno = ENOMEM;
				break;
			}
			m_fh = ::fopen( filename.c_str(), "rb");
			if (m_fh)
			{
				if (headersize_)
				{
					m_hdrbuf = std::calloc( headersize_, 1);
					if (!m_hdrbuf)
					{
						m_errno = ENOMEM;
						break;
					}
					if (1!=::fread( m_hdrbuf, headersize_, 1/*count*/, m_fh))
					{
						m_errno = ::ferror( m_fh);
						if (!m_errno) m_errno = EINVAL;
						break;
					}
				}
			}
			else
			{
				m_errno = errno;
			}
			break;
		case ExclusiveAppendWrite:
			m_fh = ::fopen( filename.c_str(), "ab");
			if (m_fh)
			{
				if (headersize_)
				{
					if (headerstruct)
					{
						if (1!=::fwrite( headerstruct, headersize_, 1/*count*/, m_fh))
						{
							m_errno = ::ferror( m_fh);
							break;
						}
					}
					else
					{
						m_errno = EINVAL;
						break;
					}
				}
			}
			else
			{
				m_errno = errno;
			}
			break;
	}
	m_mode = mode_;
	m_recordsize = recordsize_;
	m_headersize = headersize_;
	m_recordindex = 0;
	return (m_errno == 0);
}

DLL_PUBLIC const void* DataRecordFile::header()
{
	switch (m_mode)
	{
		case ExclusiveAppendWrite:
		case NoAccess:
			break;
		case SharedRead:
		{
			return m_hdrbuf;
		}
	}
	m_errno = EINVAL;
	return 0;
}

DLL_PUBLIC const void* DataRecordFile::read( std::size_t fpos)
{
	switch (m_mode)
	{
		case ExclusiveAppendWrite:
		case NoAccess:
			break;
		case SharedRead:
		{
			int origin;
			long int offset; 
			if (m_recordindex > fpos)
			{
				origin = SEEK_SET;
				offset = fpos * m_recordsize + m_headersize;
			}
			else
			{
				origin = SEEK_CUR;
				offset = (fpos - m_recordindex) * m_recordsize + m_headersize;
				if (offset == 0 && ::feof( m_fh))
				{
					return 0;
				}
			}
			if (((origin == SEEK_CUR && offset == 0) || 0==fseek( m_fh, offset, origin))
			&& 1==::fread( m_recbuf, m_recordsize, 1/*count*/, m_fh))
			{
				m_recordindex = fpos + 1;
			}
			else
			{
				m_errno = ::ferror( m_fh);
				m_recordindex = std::numeric_limits<std::size_t>::max();
				return 0;
			}
			return m_recbuf;
		}
	}
	m_errno = EINVAL;
	return 0;
}

DLL_PUBLIC const void* DataRecordFile::read()
{
	return read( m_recordindex);
}

DLL_PUBLIC bool DataRecordFile::append( const void* recbuf)
{
	switch (m_mode)
	{
		case SharedRead:
		case NoAccess:
			break;
		case ExclusiveAppendWrite:
			if (1==::fwrite( recbuf, m_recordsize, 1/*count*/, m_fh))
			{
				++m_recordindex;
			}
			else
			{
				m_errno = ::ferror( m_fh);
			}
			return true;
	}
	m_errno = EINVAL;
	return false;
}

DLL_PUBLIC bool DataRecordFile::close()
{
	if (0!=fclose( m_fh))
	{
		m_errno = ::ferror( m_fh);
	}
	else
	{
		m_errno = 0;
	}
	if (m_recbuf)
	{
		std::free( m_recbuf);
		m_recbuf = 0;
	}
	if (m_hdrbuf)
	{
		std::free( m_hdrbuf);
		m_hdrbuf = 0;
	}
	m_fh = 0;
	m_mode = NoAccess;
	m_recordsize = 0;
	m_recordindex = 0;
	m_headersize = 0;
	return (m_errno == 0);
}

DLL_PUBLIC int DataRecordFile::error() const
{
	return m_errno;
}

DLL_PUBLIC std::size_t DataRecordFile::size()
{
	switch (m_mode)
	{
		case NoAccess:
			break;
		case ExclusiveAppendWrite:
			return m_recordindex;
		case SharedRead:
		{
			if (0==::fseek( m_fh, 0, SEEK_END))
			{
				long int nn = ::ftell( m_fh);
				if (nn < 0)
				{
					m_recordindex = std::numeric_limits<std::size_t>::max();
					return 0;
				}
				else
				{
					m_recordindex = (nn / m_recordsize);
					return m_recordindex;
				}
			}
		}
	}
	m_errno = EINVAL;
	return 0;
}
#endif


