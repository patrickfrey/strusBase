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

static int mkdirp_( const std::string& dirname, std::string* firstDirectoryCreated)
{
	int ec = strus::createDir( dirname, false/*fail_ifexist*/);
	if (ec == EPERM || ec == ENOENT)
	{
		std::string parentpath;
		ec = getParentPath( dirname, parentpath);
		if (ec) return ec;
		if (parentpath.empty() || dirname.size() <= parentpath.size()) return EPERM;
		ec = mkdirp_( parentpath, firstDirectoryCreated);
		if (ec) return ec;
		ec = strus::createDir( dirname, false/*fail_ifexist*/);
		if (ec == 0 && firstDirectoryCreated && firstDirectoryCreated->empty())
		{
			firstDirectoryCreated->append( dirname);
		}
	}
	return ec;
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


DLL_PUBLIC int strus::mkdirp( const std::string& dirname)
{
	try
	{
		return mkdirp_( dirname, NULL);
	}
	catch (...)
	{
		return ENOMEM;
	}
}

DLL_PUBLIC int strus::mkdirp( const std::string& dirname, std::string& firstDirectoryCreated)
{
	try
	{
		return mkdirp_( dirname, &firstDirectoryCreated);
	}
	catch (...)
	{
		return ENOMEM;
	}
}

DLL_PUBLIC int strus::createDir( const std::string& dirname, bool fail_ifexist)
{
AGAIN:
	if (0>::mkdir( dirname.c_str(), 0755))
	{
		int ec = errno;
		if (ec == EINTR) goto AGAIN;
		if (!fail_ifexist && ec == EEXIST && isDir(dirname))
		{
			ec = 0;
		}
		return ec;
	}
	return 0;
}

DLL_PUBLIC int strus::changeDir( const std::string& dirname)
{
AGAIN:
	if (0>::chdir( dirname.c_str()))
	{
		int ec = errno;
		if (ec == EINTR) goto AGAIN;
		return ec;
	}
	return 0;
}

DLL_PUBLIC int strus::removeFile( const std::string& filename, bool fail_ifnofexist)
{
AGAIN:
	if (0>::remove( filename.c_str()))
	{
		int ec = errno;
		if (ec == EINTR) goto AGAIN;
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
AGAIN:
	if (0>::rmdir( dirname.c_str()))
	{
		int ec = errno;
		if (ec == EINTR) goto AGAIN;
		if (!fail_ifnofexist && ec == ENOENT)
		{
			ec = 0;
		}
		return ec;
	}
	return 0;
}

static int removeSubDirs_r( const std::string& dirname)
{
	std::vector<std::string> dirs;
	std::vector<std::string> files;
	int ec = 0;

	ec = readDirSubDirs( dirname, dirs);
	if (ec) return ec;
	ec = readDirFiles( dirname, "", files);
	if (ec) return ec;

	std::vector<std::string>::const_iterator fi = files.begin(), fe = files.end();
	for (; fi != fe; ++fi)
	{
		std::string fullpath = dirname + STRUS_FILEIO_DIRSEP + *fi;
		int s_ec = removeFile( fullpath);
		if (s_ec) ec = s_ec;
	}
	std::vector<std::string>::const_iterator di = dirs.begin(), de = dirs.end();
	for (; di != de; ++di)
	{
		std::string fullpath = dirname + STRUS_FILEIO_DIRSEP + *di;
		int s_ec = removeSubDirs_r( fullpath);
		if (s_ec == 0)
		{
			s_ec = removeDir( fullpath);
		}
		if (s_ec) ec = s_ec;
	}
	return ec;
}

DLL_PUBLIC int strus::removeDirRecursive( const std::string& dirname, bool fail_ifnofexist)
{
	if (isDir( dirname))
	{
		int ec = removeSubDirs_r( dirname);
		if (ec == 0)
		{
			ec = removeDir( dirname);
		}
		return ec;
	}
	else
	{
		return removeFile( dirname, fail_ifnofexist);
	}
}

DLL_PUBLIC int strus::renameFile( const std::string& old_filename, const std::string& new_filename)
{
AGAIN:
	if (0>::rename( old_filename.c_str(), new_filename.c_str()))
	{
		int ec = errno;
		if (ec == EINTR) goto AGAIN;
		return ec;
	}
	return 0;
}

static int appendFileHandle( FILE* fh, const void* content, std::size_t contentsize)
{
	char const* fi = (const char*)content;
	char const* fe = fi + contentsize;
	while (fi != fe)
	{
		std::size_t nn = fe - fi;
		std::size_t written = ::fwrite( fi, 1, nn, fh);
		if (written < nn)
		{
			int ec = ::ferror( fh);
			if (ec && ec != EINTR)
			{
				return ec;
			}
		}
		fi += written;
	}
	return 0;
}

DLL_PUBLIC int strus::writeFile( const std::string& filename, const void* content, std::size_t contentsize)
{
AGAIN:{
	FILE* fh = ::fopen( filename.c_str(), "wb");
	if (!fh)
	{
		int ec = errno;
		if (ec == EINTR) goto AGAIN;
		return ec;
	}
	int ec = appendFileHandle( fh, content, contentsize);
	::fclose( fh);
	return ec;
}}

DLL_PUBLIC int strus::writeFile( const std::string& filename, const std::string& content)
{
	return strus::writeFile( filename, content.c_str(), content.size());
}

DLL_PUBLIC int strus::appendFile( const std::string& filename, const std::string& content)
{
AGAIN:{
	FILE* fh = ::fopen( filename.c_str(), "a");
	if (!fh)
	{
		int ec = errno;
		if (ec == EINTR) goto AGAIN;
		return ec;
	}
	int ec = appendFileHandle( fh, content.c_str(), content.size());
	::fclose( fh);
	return ec;
}}

DLL_PUBLIC int strus::readFileSize( const std::string& filename, std::size_t& size)
{
AGAIN:{
	FILE* fh = ::fopen( filename.c_str(), "rb");
	if (!fh)
	{
		int ec = errno;
		if (ec == EINTR) goto AGAIN;
		return ec;
	}
	if (0 != ::fseek( fh, 0L, SEEK_END))
	{
		int ec = errno;
		if (ec == EINTR) goto AGAIN;
		::fclose( fh);
		return ec;
	}
	long filesize = size = ::ftell( fh);
	if (filesize < 0 || filesize >= std::numeric_limits<long>::max())
	{
		int ec = ::ferror( fh);
		if (!ec) ec = EISDIR;
		::fclose( fh);
		return ec;
	}
	::fclose( fh);
	return 0;
}}

DLL_PUBLIC int strus::readFile( const std::string& filename, std::string& res)
{
AGAIN:{
	FILE* fh = ::fopen( filename.c_str(), "rb");
	if (!fh)
	{
		int ec = errno;
		if (ec == EINTR) goto AGAIN;
		return ec;
	}
	if (0 != ::fseek( fh, 0L, SEEK_END))
	{
		int ec = errno;
		if (ec == EINTR) goto AGAIN;
		return ec;
	}
	long filesize = ::ftell( fh);
	if (filesize < 0 || filesize >= std::numeric_limits<long>::max())
	{
		::fclose( fh);
		return EISDIR;
	}
	if (0 != ::fseek( fh, 0L, SEEK_SET))
	{
		int ec = errno;
		if (ec == EINTR) goto AGAIN;
		return ec;
	}
	try
	{
		try
		{
			res.reserve( res.size() + filesize);
		}
		catch (const std::bad_alloc&)
		{
			::fclose( fh);
			return ENOMEM;
		}
	}
	catch (const std::bad_alloc&)
	{
		::fclose( fh);
		return ENOMEM;
	}
	unsigned int nn;
	enum {bufsize=(1<<12)};
	char buf[ bufsize];

CONTINUE_READ:
	while (!!(nn=::fread( buf, 1/*nmemb*/, bufsize, fh)))
	{
		try
		{
			res.append( buf, nn);
		}
		catch (const std::bad_alloc&)
		{
			::fclose( fh);
			return ENOMEM;
		}
	}
	if (!feof( fh))
	{
		int ec = ::ferror( fh);
		if (ec == EINTR) goto CONTINUE_READ;
		::fclose( fh);
		return ec;
	}
	else
	{
		::fclose( fh);
	}
	return 0;
}}

DLL_PUBLIC int strus::readStdin( std::string& res)
{
	unsigned int nn;
	enum {bufsize=(1<<12)};
	char buf[ bufsize];

CONTINUE_READ:
	while (!!(nn=::fread( buf, 1/*nmemb*/, bufsize, stdin)))
	{
		try
		{
			res.append( buf, nn);
		}
		catch (const std::bad_alloc&)
		{
			return ENOMEM;
		}
	}
	int ec = ::ferror( stdin);
	if (ec == EINTR) goto CONTINUE_READ;
	return ec;
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
			std::string entry( path + STRUS_FILEIO_DIRSEP + ent->d_name);
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
		return ENOMEM;
	}
	return 0;
}

DLL_PUBLIC int strus::readDirItems( const std::string& path, std::vector<std::string>& res)
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
			std::string entry( path + STRUS_FILEIO_DIRSEP + ent->d_name);
			res.push_back( ent->d_name);
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
		return ENOMEM;
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
			std::string entrypath( path + STRUS_FILEIO_DIRSEP + entry);
			if (isDir( entrypath)) continue;
			if (ext.empty())
			{
				res.push_back( entry);
			}
			else
			{
				const char* ee = entry.c_str() + entry.size() - ext.size();
				if (0==std::memcmp( ee, ext.c_str(), ext.size()))
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
		return ENOMEM;
	}
}

static bool matchFileNamePattern( const char* pt, const char* name)
{
	char const* pi = pt;
	char const* ni = name;

	while (*pi && *ni)
	{
		if (*pi == '*')
		{
			++pi;
			if (!*pi) return true;

			char const* gi = ni;
			for (; *gi; ++gi)
			{
				if (matchFileNamePattern( pi, gi)) return true;
			}
			return false;
		}
		if (*pi == '?' || *pi == *ni)
		{
			++pi;
			++ni;
		}
		else
		{
			return false;
		}
	}
	if (*pi)
	{
		while (*pi == '*') ++pi;
	}
	return (!*pi && !*ni);
}

static int expandFilePattern_( const std::string& pathPattern, std::vector<std::string>& res)
{
	char const* xi = std::strchr( pathPattern.c_str(), '*');
	char const* yi = std::strchr( pathPattern.c_str(), '?');
	if (!xi && !yi)
	{
		PathType pt = getPathType( pathPattern);
		if (pt == PathFile || pt == PathDir)
		{
			res.push_back( pathPattern);
		}
		return 0;
	}
	char const* pi;
	char const* pe;
	if (yi && xi) pi = (yi < xi) ? yi : xi; else pi = xi ? xi : yi;
	pe = pi + 1;
	for (; pi != pathPattern.c_str() && *(pi-1) != STRUS_FILEIO_DIRSEP; --pi){}
	for (; *pe && *pe != STRUS_FILEIO_DIRSEP; ++pe){}

	std::string itemPattern( pi, pe-pi);
	std::string path;
	if (pi == pathPattern.c_str())
	{
		path.append( ".");
	}
	else
	{
		path.append( pathPattern.c_str(), pi - pathPattern.c_str() - 1);
	}
	std::vector<std::string> items;
	int ec = readDirItems( path, items);
	if (ec) return ec;

	std::vector<std::string>::const_iterator ti = items.begin(), te = items.end();
	for (; ti != te; ++ti)
	{
		if (matchFileNamePattern( itemPattern.c_str(), ti->c_str()))
		{
			std::string expandPattern = strus::joinFilePath( path, *ti);
			if (*pe) expandPattern = strus::joinFilePath( expandPattern, pe+1);
			ec = expandFilePattern_( expandPattern, res);
			if (ec) return ec;
		}
	}
	return 0;
}

DLL_PUBLIC int strus::expandFilePattern( const std::string& pathPattern, std::vector<std::string>& res)
{
	try
	{
		return expandFilePattern_( pathPattern, res);
	}
	catch (const std::bad_alloc&)
	{
		return ENOMEM;
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

static bool isExplicitLocalPath( const std::string& path)
{
	if (path.size() > 1 && path[0] == '.')
	{
		if (path[1] == STRUS_FILEIO_DIRSEP) return true;
		if (path.size() > 2 && path[1] == '.')
		{
			if (path[2] == STRUS_FILEIO_DIRSEP) return true;
		}
	}
	return false;
}

DLL_PUBLIC bool strus::isRelativePath( const std::string& path)
{
	return !path.empty() && path[0] != STRUS_FILEIO_DIRSEP && !isExplicitLocalPath( path);
}

DLL_PUBLIC bool strus::isAbsolutePath( const std::string& path)
{
	return !path.empty() && path[0] == STRUS_FILEIO_DIRSEP;
}

DLL_PUBLIC bool strus::isExplicitPath( const std::string& path)
{
	return !path.empty() && (path[0] == STRUS_FILEIO_DIRSEP || isExplicitLocalPath( path));
}

DLL_PUBLIC char strus::dirSeparator()
{
	return STRUS_FILEIO_DIRSEP;
}

DLL_PUBLIC int strus::getAncestorPath( const std::string& path, int level, std::string& dest)
{
	try
	{
		char const* pt = path.c_str();
		std::string pathbuf;
		if (pt[0] == '.')
		{
			char cwd[2048];
			const char* pwd = ::getcwd(cwd, sizeof(cwd));
			if (!pwd) return ENOMEM;
			pathbuf.append( pwd);
			pathbuf.push_back( STRUS_FILEIO_DIRSEP);
			pathbuf.append( pt);
			pt = pathbuf.c_str();
		}
		char const* se = std::strchr( pt, '\0');
		char const* si = pt;
		while (se > si && *(se-1) == STRUS_FILEIO_DIRSEP) --se;
		while (se > si && level)
		{
			char const* pi = se;
			while (pi > si && *(pi-1) != STRUS_FILEIO_DIRSEP) --pi;
			if (pi == si)
			{
				break;
			}
			if (se-pi == 1 && pi[0] == '.')
			{
				se = pi;
				while (se > si && *(se-1) == STRUS_FILEIO_DIRSEP) --se;
				continue;
			}
			if (se-pi == 2 && pi[0] == '.' && pi[1] == '.')
			{
				se = pi;
				while (se > si && *(se-1) == STRUS_FILEIO_DIRSEP) --se;
				level += 1;
				continue;
			}
			se = pi;
			while (se > si && *(se-1) == STRUS_FILEIO_DIRSEP) --se;
			--level;
		}
		if (level) return EINVAL;
		dest = std::string( si, se-si);
		return 0;
	}
	catch (const std::bad_alloc&)
	{
		return ENOMEM;
	}
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
			dest.append( ri, re-ri);
		}
		catch (const std::bad_alloc&)
		{
			return ENOMEM;
		}
		return 0;
	}
}

DLL_PUBLIC int strus::getFileName( const std::string& path, std::string& dest, bool withExtension)
{
	const char* ri = path.c_str();
	char const* re = path.c_str()+path.size();
	for (; re != ri && *(re-1) == STRUS_FILEIO_DIRSEP; --re){}
	char const* namend = re;
	if (!withExtension)
	{
		for (; re != ri && *(re-1) != STRUS_FILEIO_DIRSEP && *(re-1) != '.'; --re){}
		if (re != ri && *(re-1) == '.')
		{
			namend = re-1;
		}
	}
	for (; re != ri && *(re-1) != STRUS_FILEIO_DIRSEP; --re){}
	try
	{
		dest.clear();
		dest.append( re, namend - re);
	}
	catch (const std::bad_alloc&)
	{
		return ENOMEM;
	}
	return 0;
}

DLL_PUBLIC int strus::getFileExtension( const std::string& path, std::string& ext)
{
	const char* ri = path.c_str();
	char const* re = path.c_str()+path.size();
	ext.clear();
	for (; re != ri && *(re-1) == STRUS_FILEIO_DIRSEP; --re){}
	for (; re != ri && *(re-1) != STRUS_FILEIO_DIRSEP && *(re-1) != '.'; --re){}
	if (re == ri || *(re-1) == STRUS_FILEIO_DIRSEP)
	{}
	else if (*(re-1) == '.')
	{
		--re;	//... include '.' in result
		try
		{
			ext.append( re);
		}
		catch (const std::bad_alloc&)
		{
			return ENOMEM;
		}
	}
	return 0;
}

DLL_PUBLIC bool strus::hasUpdirReference( const std::string& path)
{
	char const* ri = path.c_str();
	char const* re = path.c_str()+path.size();
	if (path.size() < 2) return false;
	if (ri[0] == '.' && ri[1] == '.') return true;

	for (;ri != re; ++ri)
	{
		if (ri[0] == STRUS_FILEIO_DIRSEP && ri[1] == '.' && ri[2] == '.') return true;
	}
	return false;
}


DLL_PUBLIC std::string strus::joinFilePath( const std::string& parentpath, const std::string& childpath)
{
	try
	{
		if (parentpath.empty())
		{
			char const* ri = childpath.c_str();
			if (*ri != STRUS_FILEIO_DIRSEP) return childpath;
		}
		const char* pi = parentpath.c_str();
		char const* pe = pi + parentpath.size();
		while (pe != pi && *(pe-1) == STRUS_FILEIO_DIRSEP) --pe;
		char const* ri = childpath.c_str();
		while (*ri == STRUS_FILEIO_DIRSEP) ++ri;
		return std::string(pi,pe-pi) + STRUS_FILEIO_DIRSEP + ri;
	}
	catch (const std::bad_alloc&)
	{
		return std::string();
	}
}

DLL_PUBLIC int strus::resolveUpdirReferences( std::string& path)
{
	try
	{
		char ptstart = '\0';
		std::vector<int> dirstarts;
		char const* pi = path.c_str();
		if (*pi == STRUS_FILEIO_DIRSEP)
		{
			for (++pi; *pi == STRUS_FILEIO_DIRSEP; ++pi){}
			ptstart = STRUS_FILEIO_DIRSEP;
			if (*pi) dirstarts.push_back( pi - path.c_str());
		}
		else if (*pi == '.')
		{
			if (pi[1] == STRUS_FILEIO_DIRSEP)
			{
				for (++pi; *pi == STRUS_FILEIO_DIRSEP; ++pi){}
				ptstart = '.';
				dirstarts.push_back( pi - path.c_str());
			}
			else if (pi[1] == '.' && (pi[2] == STRUS_FILEIO_DIRSEP || pi[2] == '\0'))
			{
				return EINVAL;
			}
			else
			{
				dirstarts.push_back( 0);
			}
		}
		else
		{
			dirstarts.push_back( 0);
		}
		for (; *pi; ++pi)
		{
			if (*pi == STRUS_FILEIO_DIRSEP)
			{
				while (pi[1] == STRUS_FILEIO_DIRSEP) ++pi;
				if (pi[1] == '.' && pi[2] == '.' && (pi[3] == STRUS_FILEIO_DIRSEP || pi[3] == '\0'))
				{
					pi += 2;
					if (dirstarts.empty()) return EINVAL;
					dirstarts.pop_back();
				}
				else if (pi[1] == '.' && (pi[2] == STRUS_FILEIO_DIRSEP || pi[2] == '\0'))
				{
					pi += 1;
					/// ... self reference "/./", skip
				}
				else
				{
					dirstarts.push_back( pi - path.c_str() + 1);
				}
			}
		}
		std::string respath;
		if (ptstart == STRUS_FILEIO_DIRSEP)
		{
			respath.append( "/");
		}
		else if (ptstart == '.')
		{
			respath.append( "./");
		}
		std::vector<int>::const_iterator di = dirstarts.begin(), de = dirstarts.end();
		for (; di != de; ++di)
		{
			pi = path.c_str() + *di;
			for (; *pi && *pi != STRUS_FILEIO_DIRSEP; ++pi)
			{
				respath.push_back( *pi);
			}
			if (*pi == STRUS_FILEIO_DIRSEP)
			{
				respath.push_back( STRUS_FILEIO_DIRSEP);
			}
		}
		path = respath;
		return 0/*OK*/;
	}
	catch (const std::bad_alloc&)
	{
		return ENOMEM;
	}
}

