/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Basic portable file read/write and related filesystem functions
#ifndef _STRUS_BASE_FILE_IO_HPP_INCLUDED
#define _STRUS_BASE_FILE_IO_HPP_INCLUDED
#include <vector>
#include <string>
#include <cstdio>

namespace strus
{

/// \brief Reads the complete content of a file to a string
/// \param[in] filename the name of file to read
/// \param[out] res the returned content of the file read
/// \return 0 on success, errno on failure
unsigned int readFile( const std::string& filename, std::string& res);

/// \brief Reads the complete input of stdin to a string
/// \param[out] res the returned content of the file read
/// \return 0 on success, errno on failure
unsigned int readStdin( std::string& res);

/// \brief Writes a string to a file creating the file if it does not exist
/// \param[in] filename the name of the file to write
/// \param[out] content content of the string to write
/// \return 0 on success, errno on failure
unsigned int writeFile( const std::string& filename, const std::string& content);

/// \brief Removes a file
/// \param[in] filename the name of the file to remove
/// \param[in] fail_ifnofexist return an error if the directory does not exist
/// \return 0 on success, errno on failure
unsigned int removeFile( const std::string& filename, bool fail_ifnofexist=false);

/// \brief Removes an empty directory
/// \param[in] dirname the name of the directory to remove
/// \param[in] fail_ifnofexist return an error if the directory does not exist
/// \return 0 on success, errno on failure
unsigned int removeDir( const std::string& dirname, bool fail_ifnofexist=false);

/// \brief Creates a directory if it does not exist yet
/// \param[in] dirname path of directory to create
/// \param[in] fail_ifexist return an error if the directory already exists
unsigned int createDir( const std::string& dirname, bool fail_ifexist=true);

/// \brief Reads all file names matching to the extension ext of a directory to an array of strings
/// \param[in] path the name of director to read
/// \param[in] ext the file extension the accepted files must have (empty if all extensions are accepted)
/// \param[out] res the returned matching file names of the directory read
/// \return 0 on success, errno on failure
unsigned int readDirFiles( const std::string& path, const std::string& ext, std::vector<std::string>& res);

/// \brief Reads all subdirectories of a directory to an array of strings
/// \param[in] path the name of director to read
/// \param[out] res the returned matching sub directories of the directory read
/// \return 0 on success, errno on failure
unsigned int readDirSubDirs( const std::string& path, std::vector<std::string>& res);

/// \brief Check if a file system path points to a file
/// \return true, if yes
/// \param[in] path file system path to check
bool isFile( const std::string& path);

/// \brief Check if a file system path points to a directory
/// \param[in] path file system path to check
/// \return true, if yes
bool isDir( const std::string& path);

/// \brief Get the parent (containing) path of a file or directory
/// \param[in] path file system path to get the containing path
/// \param[out] dest the containing path
/// \return 0 on success, errno on failure
unsigned int getParentPath( const std::string& path, std::string& dest);

/// \brief Get the OS path element separator
/// \return the separator
char dirSeparator();


/// \brief Abstraction for data record files
#ifdef _MSC_VER
class DataRecordFile;
//... Posix file functions do not open a file in shared read mode in Windows.
//    Therefore we need an own implementation with Win file functions that is not available yet.
#else
/// \remark Implementaion is not thread safe
class DataRecordFile
{
public:
	/// \brief Implemented modes to access a data record file
	enum Mode {NoAccess=0x0, SharedRead=0x1, ExclusiveAppendWrite=0x4};

public:
	///\brief Constructor
	DataRecordFile();
	///\brief Destructor
	~DataRecordFile();

	///\brief Open a file
	///\param[in] filename name of the file o open
	///\param[in] mode_ file mode describing the access to the file
	///\param[in] recordsize_ size of one record in bytes
	///\return true, if success, false on failure (see error() for the system error code ~ errno)
	bool open( const std::string& filename, const Mode& mode_, unsigned int recordsize_);

	///\brief Read one record of the file
	///\param[in] fpos index of the record to read (file position is the index multiplied with the record size)
	///\return pointer to buffer read if success, (0) NULL on failure (see error() for the system error code ~ errno)
	const void* read( std::size_t fpos);

	///\brief Append a record at the end of the file
	///\param[in] recbuf buffer containing the record to write
	///\return true, if success, false on failure (see error() for the system error code ~ errno)
	bool append( const void* recbuf);

	///\brief Close the file
	///\return true, if success, false on failure (see error() for the system error code ~ errno)
	bool close();

	///\brief Get the error of the last operation to this file
	///\return the error code (system errno)
	int error() const;

	///\brief Get the size of one record of this file
	std::size_t recordsize() const		{return m_recordsize;}

	///\brief Get the number of records stored in the file
	std::size_t size();

private:
	FILE* m_fh;			///< file handle
	Mode m_mode;			///< file access mode
	int m_errno;			///< system errno for this file
	unsigned int m_recordsize;	///< size of a record in bytes
	std::size_t m_recordindex;	///< current file position
	void* m_recbuf;

private:
	DataRecordFile( const DataRecordFile&){}				///< non copyable
	DataRecordFile& operator=( const DataRecordFile&){return *this;}	///< non copyable
};
#endif

}//namespace
#endif

