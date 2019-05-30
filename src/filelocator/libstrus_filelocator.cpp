/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Standard filelocator implementation
/// \file libstrus_filelocator.cpp
#include "strus/lib/filelocator.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/fileio.hpp"
#include "fileio_sys.hpp"
#include "strus/base/string_conv.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>

using namespace strus;

class FileLocator
	:public FileLocatorInterface
{
public:
	FileLocator( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_)
	{
		DebugTraceInterface* dbg = m_errorhnd->debugTrace();
		m_debugtrace = (dbg) ? dbg->createTraceContext( "file") : NULL;
	}

	virtual ~FileLocator()
	{
		if (m_debugtrace) delete m_debugtrace;
	}

	virtual void addResourcePath( const std::string& path)
	{
		try
		{
			char const* cc = path.c_str();
			char const* ee = std::strchr( cc, STRUS_FILEIO_PATHSEP);
			for (; ee!=0; cc=ee+1,ee=std::strchr( cc, STRUS_FILEIO_PATHSEP))
			{
				addResourcePathPart( string_conv::trim( std::string( cc, ee)));
			}
			addResourcePathPart( string_conv::trim( std::string( cc)));
		}
		CATCH_ERROR_ARG1_MAP( _TXT("error in '%s': %s"), "FileLocator::addResourcePath", *m_errorhnd);
	}

	virtual std::string getResourceFilePath( const std::string& filename) const
	{
		try
		{
			if (strus::hasUpdirReference( filename))
			{
				m_errorhnd->report( ErrorCodeNotAllowed, _TXT("tried to read resource file with upper directory reference in the name"));
				return std::string();
			}
			if (m_debugtrace) m_debugtrace->event( "search-resource", "filename '%s'", filename.c_str());
			std::vector<std::string>::const_iterator
				pi = m_resourcePaths.begin(), pe = m_resourcePaths.end();
			for (; pi != pe; ++pi)
			{
				std::string absfilename = strus::joinFilePath( *pi, filename);
				if (m_debugtrace) m_debugtrace->event( "search-path", "path '%s'", pi->c_str());
				if (strus::isFile( absfilename) || strus::isDir( absfilename))
				{
					if (m_debugtrace) m_debugtrace->event( "found-resource", "path '%s'", absfilename.c_str());
					return absfilename;
				}
			}
			throw strus::runtime_error( _TXT("resource file '%s' not found"), filename.c_str());
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s': %s"), "FileLocator::getResourcePath", *m_errorhnd, std::string());
	}

	virtual void defineWorkingDirectory( const std::string& path)
	{
		try
		{
			if (m_debugtrace) m_debugtrace->event( "workdir", "path %s", path.c_str());
			if (!m_workDir.empty() || 0!=std::strchr( path.c_str(), STRUS_FILEIO_PATHSEP)) throw std::runtime_error(_TXT("multiple definitions of working directory"));
			m_workDir = path;
		}
		CATCH_ERROR_ARG1_MAP( _TXT("error in '%s': %s"), "FileLocator::defineWorkingDirectory", *m_errorhnd);
	}

	virtual std::string getWorkingDirectory() const
	{
		try
		{
			return m_workDir;
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s': %s"), "FileLocator::getWorkDir", *m_errorhnd, std::string());
	}

	virtual std::vector<std::string> getResourcePaths() const
	{
		try
		{
			return m_resourcePaths;
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s': %s"), "FileLocator::getResourcePaths", *m_errorhnd, std::vector<std::string>());
	}

private:
	void addResourcePathPart( const std::string& path)
	{
		m_resourcePaths.push_back( path);
		if (m_debugtrace) m_debugtrace->event( "resourcedir", "path %s", path.c_str());
	}

private:
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
	std::vector<std::string> m_resourcePaths;
	std::string m_workDir;
};


DLL_PUBLIC FileLocatorInterface* strus::createFileLocator_std( ErrorBufferInterface* errorhnd)
{
	try
	{
		return new FileLocator( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating file locator: %s"), *errorhnd, NULL);
}

