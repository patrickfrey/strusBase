/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/fileio.hpp"
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
#include <algorithm>
#include <functional>
#include <limits>
#include <errno.h>

using namespace strus;

DLL_PUBLIC unsigned int strus::createDir( const std::string& dirname, bool fail_ifexist)
{
	if (0>::mkdir( dirname.c_str(), 0755))
	{
		unsigned int ec = errno;
		if (!fail_ifexist && ec == EEXIST && isDir(dirname.c_str()))
		{
			ec = 0;
		}
		return ec;
	}
	return 0;
}

DLL_PUBLIC unsigned int strus::removeFile( const std::string& filename, bool fail_ifnofexist)
{
	if (0>::remove( filename.c_str()))
	{
		unsigned int ec = errno;
		if (!fail_ifnofexist && ec == ENOENT)
		{
			ec = 0;
		}
		return ec;
	}
	return 0;
}

DLL_PUBLIC unsigned int strus::removeDir( const std::string& dirname, bool fail_ifnofexist)
{
	if (0>::rmdir( dirname.c_str()))
	{
		unsigned int ec = errno;
		if (!fail_ifnofexist && ec == ENOENT)
		{
			ec = 0;
		}
		return ec;
	}
	return 0;
}

DLL_PUBLIC unsigned int strus::writeFile( const std::string& filename, const std::string& content)
{
	unsigned char ch;
	FILE* fh = ::fopen( filename.c_str(), "wb");
	if (!fh)
	{
		return errno;
	}
	std::string::const_iterator fi = content.begin(), fe = content.end();
	for (; fi != fe; ++fi)
	{
		ch = *fi;
		if (1 > ::fwrite( &ch, 1, 1, fh))
		{
			int ec = ::ferror( fh);
			if (ec)
			{
				::fclose( fh);
				return ec;
			}
		}
	}
	::fclose( fh);
	return 0;
}

DLL_PUBLIC unsigned int strus::readFile( const std::string& filename, std::string& res)
{
	FILE* fh = ::fopen( filename.c_str(), "rb");
	if (!fh)
	{
		return errno;
	}
	::fseek( fh, 0L, SEEK_END);
	std::size_t filesize = ::ftell( fh);
	::fseek( fh, 0L, SEEK_SET);
	try
	{
		res.reserve( res.size() + filesize);
	}
	catch (const std::bad_alloc&)
	{
		::fclose( fh);
		return 12/*ENOMEM*/;
	}
	unsigned int nn;
	enum {bufsize=(1<<12)};
	char buf[ bufsize];

	while (!!(nn=::fread( buf, 1, bufsize, fh)))
	{
		try
		{
			res.append( buf, nn);
		}
		catch (const std::bad_alloc&)
		{
			::fclose( fh);
			return 12/*ENOMEM*/;
		}
	}
	if (!feof( fh))
	{
		unsigned int ec = ::ferror( fh);
		::fclose( fh);
		return ec;
	}
	else
	{
		::fclose( fh);
	}
	return 0;
}

DLL_PUBLIC unsigned int strus::readStdin( std::string& res)
{
	unsigned int nn;
	enum {bufsize=(1<<12)};
	char buf[ bufsize];

	while (!!(nn=::fread( buf, 1, bufsize, stdin)))
	{
		try
		{
			res.append( buf, nn);
		}
		catch (const std::bad_alloc&)
		{
			return 12/*ENOMEM*/;
		}
	}
	return 0;
}

DLL_PUBLIC unsigned int strus::readDirSubDirs( const std::string& path, std::vector<std::string>& res)
{
	DIR *dir = ::opendir( path.c_str());
	struct dirent *ent;

	if (!dir)
	{
		return errno;
	}
	try
	{
		std::size_t prevsize = res.size();
		while (!!(ent = ::readdir(dir)))
		{
			if (ent->d_name[0] == '.') continue;
			std::string entry( path + dirSeparator() + ent->d_name);
			if (isDir( entry))
			{
				res.push_back( ent->d_name);
			}
		}
		std::sort( res.begin()+prevsize, res.end(), std::less<std::string>());
		unsigned int err = ::closedir(dir);
		if (err)
		{
			return err;
		}
	}
	catch (const std::bad_alloc&)
	{
		::closedir(dir);
		return 12/*ENOMEM*/;
	}
	return 0;
}

DLL_PUBLIC unsigned int strus::readDirFiles( const std::string& path, const std::string& ext, std::vector<std::string>& res)
{
	DIR *dir = ::opendir( path.c_str());
	struct dirent *ent;

	if (!dir)
	{
		return errno;
	}
	try
	{
		std::size_t prevsize = res.size();
		while (!!(ent = ::readdir(dir)))
		{
			if (ent->d_name[0] == '.') continue;
			std::string entry( ent->d_name);
			if (ext.size() > entry.size())
			{
				continue;
			}
			std::string entrypath( path + dirSeparator() + entry);
			if (isDir( entrypath)) continue;
			if (ext.empty())
			{
				res.push_back( entry);
			}
			else
			{
				const char* ee = entry.c_str() + entry.size() - ext.size();
				if (entry[0] != '.' && 0==std::memcmp( ee, ext.c_str(), ext.size()))
				{
					res.push_back( entry );
				}
			}
		}
		std::sort( res.begin()+prevsize, res.end(), std::less<std::string>());
		unsigned int err = ::closedir(dir);
		return err;
	}
	catch (const std::bad_alloc&)
	{
		::closedir(dir);
		return 12/*ENOMEM*/;
	}
}

enum PathType {PathFile,PathDir,PathUnknown,PathError};
static PathType getPathType( const std::string& path)
{
	struct stat s;
	if (::stat( path.c_str(), &s) == 0)
	{
		if( s.st_mode & S_IFDIR )
		{
			return PathDir;
		}
		else if( s.st_mode & S_IFREG )
		{
			return PathFile;
		}
		else
		{
			return PathUnknown;
		}
	}
	else
	{
		return PathError;
	}
}

DLL_PUBLIC bool strus::isFile( const std::string& path)
{
	return getPathType(path)==PathFile;
}

DLL_PUBLIC bool strus::isDir( const std::string& path)
{
	return getPathType(path)==PathDir;
}

DLL_PUBLIC char strus::dirSeparator()
{
	return STRUS_FILEIO_DIRSEP;
}

DLL_PUBLIC unsigned int strus::getParentPath( const std::string& path, std::string& dest)
{
	const char* ri = path.c_str();
	char const* re = path.c_str()+path.size();
	for (; re != ri && *(re-1) == STRUS_FILEIO_DIRSEP; --re){}
	for (; re != ri && *(re-1) != STRUS_FILEIO_DIRSEP; --re){}
	if (re == ri)
	{
		dest.clear();
		return 0;
	}
	else
	{
		for (; re != ri && *(re-1) == STRUS_FILEIO_DIRSEP; --re){}
		try
		{
			dest.clear();
			dest.append( ri, re-ri-1);
		}
		catch (const std::bad_alloc&)
		{
			return 12/*ENOMEM*/;
		}
		dest.clear();
		return 0;
	}
}


/// \brief Abstraction for data record files
#ifdef _MSC_VER
class DataRecordFile {};
#else
DLL_PUBLIC DataRecordFile::DataRecordFile()
	:m_fh(0),m_mode(NoAccess),m_errno(0),m_recordsize(0),m_recordindex(0),m_recbuf(0)
{}

DLL_PUBLIC DataRecordFile::~DataRecordFile()
{
	(void)close();
}

DLL_PUBLIC bool DataRecordFile::open( const std::string& filename, const Mode& mode_, unsigned int recordsize_)
{
	if (m_fh && !close()) return false;
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
				return false;
			}
			m_fh = ::fopen( filename.c_str(), "rb");
			if (!m_fh) m_errno = errno;
			break;
		case ExclusiveAppendWrite:
			m_fh = ::fopen( filename.c_str(), "ab");
			if (!m_fh) m_errno = errno;
			break;
	}
	m_mode = mode_;
	m_recordsize = recordsize_;
	m_recordindex = 0;
	return (m_errno == 0);
}

DLL_PUBLIC void* DataRecordFile::read( std::size_t fpos)
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
				offset = fpos * m_recordsize;
			}
			else
			{
				origin = SEEK_CUR;
				offset = (fpos - m_recordindex) * m_recordsize;
			}
			if (((origin == SEEK_CUR && offset == 0) || 0==fseek( m_fh, offset, origin))
			&& 1==fread( m_recbuf, m_recordsize, 1/*count*/, m_fh))
			{
				m_recordindex = fpos + 1;
			}
			else
			{
				m_errno = ferror( m_fh);
				m_recordindex = std::numeric_limits<std::size_t>::max();
				return 0;
			}
			return m_recbuf;
		}
	}
	m_errno = EINVAL;
	return 0;
}

DLL_PUBLIC bool DataRecordFile::append( const void* recbuf)
{
	switch (m_mode)
	{
		case SharedRead:
		case NoAccess:
			break;
		case ExclusiveAppendWrite:
			if (1==fwrite( recbuf, m_recordsize, 1/*count*/, m_fh))
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
	m_fh = 0;
	m_mode = NoAccess;
	m_recordsize = 0;
	m_recordindex = 0;
	return (m_errno == 0);
}

DLL_PUBLIC int DataRecordFile::error() const
{
	return m_errno;
}

#endif

