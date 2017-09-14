/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/fileio.hpp"
#include "strus/base/utf8.hpp"
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

using namespace strus;

DLL_PUBLIC int strus::createDir( const std::string& dirname, bool fail_ifexist)
{
	if (0>::mkdir( dirname.c_str(), 0755))
	{
		int ec = errno;
		if (!fail_ifexist && ec == EEXIST && isDir(dirname.c_str()))
		{
			ec = 0;
		}
		return ec;
	}
	return 0;
}

DLL_PUBLIC int strus::removeFile( const std::string& filename, bool fail_ifnofexist)
{
	if (0>::remove( filename.c_str()))
	{
		int ec = errno;
		if (!fail_ifnofexist && ec == ENOENT)
		{
			ec = 0;
		}
		return ec;
	}
	return 0;
}

DLL_PUBLIC int strus::removeDir( const std::string& dirname, bool fail_ifnofexist)
{
	if (0>::rmdir( dirname.c_str()))
	{
		int ec = errno;
		if (!fail_ifnofexist && ec == ENOENT)
		{
			ec = 0;
		}
		return ec;
	}
	return 0;
}

DLL_PUBLIC int strus::writeFile( const std::string& filename, const std::string& content)
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

DLL_PUBLIC int strus::appendFile( const std::string& filename, const std::string& content)
{
	unsigned char ch;
	FILE* fh = ::fopen( filename.c_str(), "a");
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

DLL_PUBLIC int strus::readFileSize( const std::string& filename, std::size_t& size)
{
	int ec = 0;
	FILE* fh = ::fopen( filename.c_str(), "rb");
	if (!fh)
	{
		return errno;
	}
	::fseek( fh, 0L, SEEK_END);
	long filesize = size = ::ftell( fh);
	if (filesize < 0 || filesize >= std::numeric_limits<long>::max())
	{
		ec = ::ferror( fh);
		if (!ec) ec = 21/*EISIDR*/;
	}
	::fclose( fh);
	return ec;
}

DLL_PUBLIC int strus::readFile( const std::string& filename, std::string& res)
{
	FILE* fh = ::fopen( filename.c_str(), "rb");
	if (!fh)
	{
		return errno;
	}
	::fseek( fh, 0L, SEEK_END);
	long filesize = ::ftell( fh);
	if (filesize < 0 || filesize >= std::numeric_limits<long>::max())
	{
		::fclose( fh);
		return 21/*EISIDR*/;
	}
	::fseek( fh, 0L, SEEK_SET);
	try
	{
		try
		{
			res.reserve( res.size() + filesize);
		}
		catch (const std::bad_alloc&)
		{
			::fclose( fh);
			return 12/*ENOMEM*/;
		}
	}
	catch (const std::bad_alloc&)
	{
		::fclose( fh);
		return 12/*ENOMEM*/;
	}
	unsigned int nn;
	enum {bufsize=(1<<12)};
	char buf[ bufsize];

	while (!!(nn=::fread( buf, 1/*nmemb*/, bufsize, fh)))
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
		int ec = ::ferror( fh);
		::fclose( fh);
		return ec;
	}
	else
	{
		::fclose( fh);
	}
	return 0;
}

DLL_PUBLIC int strus::readStdin( std::string& res)
{
	unsigned int nn;
	enum {bufsize=(1<<12)};
	char buf[ bufsize];

	while (!!(nn=::fread( buf, 1/*nmemb*/, bufsize, stdin)))
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

DLL_PUBLIC bool strus::isTextFile( const std::string& path)
{
	unsigned char sample[ 4096];
	unsigned int count[ 256];
	std::memset( count, 0, sizeof( count));

	FILE* fh = ::fopen( path.c_str(), "rb");
	if (!fh)
	{
		return false;
	}
	unsigned int samplesize = ::fread( sample, 1/*nmemb*/, sizeof(sample), fh);
	if (samplesize < sizeof(sample) && !feof( fh))
	{
		return false;
	}
	else
	{
		::fclose( fh);
	}
	unsigned int fi=0,fe=samplesize;
	for (; fi != fe; ++fi)
	{
		++count[ sample[ fi]];
	}
	unsigned int cntLatinAlpha = 0;
	unsigned int cntTextControl = 0;
	unsigned int utf8errors = 0;
	unsigned char ci,ce;
	ci='A', ce='Z';
	for (; ci != ce; ++ci) cntLatinAlpha += count[ ci];
	ci='a', ce='z';
	for (; ci != ce; ++ci) cntLatinAlpha += count[ ci];
	ci='0', ce='9';
	for (; ci != ce; ++ci) cntLatinAlpha += count[ ci];
	cntTextControl += count[ (unsigned char)' '];
	cntTextControl += count[ (unsigned char)'\t'];
	cntTextControl += count[ (unsigned char)'\n'];
	cntTextControl += count[ (unsigned char)'\r'];
	for (fi=0; fi != fe;)
	{
		unsigned int charlen = utf8charlen( sample[fi]);
		if (charlen == 1)
		{
			if (sample[fi] > 127)
			{
				++utf8errors;
			}
			++fi;
		}
		else
		{
			for (++fi,--charlen; fi != fe && charlen > 0; ++fi,--charlen)
			{
				if (!utf8midchr( sample[fi]))
				{
					++utf8errors;
					break;
				}
			}
		}
	}
	int vote = 0;
	vote += (utf8errors > samplesize / 200) ? -1:+1;
	vote += (cntTextControl < samplesize / 20) ? -1:+1;
	vote += (cntLatinAlpha < samplesize / 2) ? -1:+1;
	return vote > 0;
}

DLL_PUBLIC int strus::readDirSubDirs( const std::string& path, std::vector<std::string>& res)
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
#ifdef __GNUC__
			if (ent->d_type == DT_LNK) continue; //... do not follow symbolic links
#endif
			if (ent->d_name[0] == '.') continue;
			std::string entry( path + dirSeparator() + ent->d_name);
			if (isDir( entry))
			{
				res.push_back( ent->d_name);
			}
		}
		std::sort( res.begin()+prevsize, res.end(), std::less<std::string>());
		int err = ::closedir(dir);
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

DLL_PUBLIC int strus::readDirFiles( const std::string& path, const std::string& ext, std::vector<std::string>& res)
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
		int err = ::closedir(dir);
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

DLL_PUBLIC int strus::getParentPath( const std::string& path, std::string& dest)
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


