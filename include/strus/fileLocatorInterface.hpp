/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for locating resource files and working directory
/// \file fileLocatorInterface.hpp
#ifndef _STRUS_BASE_FILELOCATOR_INTERFACE_HPP_INCLUDED
#define _STRUS_BASE_FILELOCATOR_INTERFACE_HPP_INCLUDED
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus
{

/// \class FileLocatorInterface
/// \brief Interface for locating resource files and working directory
class FileLocatorInterface
{
public:
	/// \brief Desructor
	virtual ~FileLocatorInterface(){}

	/// \brief Declare a path for locating resource files
	/// \param[in] path path to add
	virtual void addResourcePath( const std::string& path)=0;

	/// \brief Get the absolute path of a resource file
	/// \param[in] filename name of the resource file
	/// \return the full path of the file with the filename passed as argument
	virtual std::string getResourcePath( const std::string& filename) const=0;

	/// \brief Define the working directory where to write files to
	/// \param[in] workdir directory path specifying the root directory of all files written
	virtual void defineWorkDir( const std::string& workdir)=0;

	/// \brief Get the current working directory, the root directory of files and directories written
	/// \return the working directory
	virtual std::string getWorkDir() const=0;
};

}//namespace
#endif

