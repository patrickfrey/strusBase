/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Basic portable file type for collections of equal sized records with random access
#ifndef _STRUS_BASE_DATA_RECORD_FILE_HPP_INCLUDED
#define _STRUS_BASE_DATA_RECORD_FILE_HPP_INCLUDED
#include "strus/base/fileio.hpp"
#include <vector>
#include <string>
#include <cstdio>

namespace strus
{

/// \brief Basic portable file type for collections of equal sized records with random access
#ifdef _MSC_VER
class DataRecordFile;
//... Posix file functions do not open a file in shared read mode in Windows.
//    Therefore we need an own implementation with Win file functions that is not available yet.
#else
/// \remark Access of the same file with the same DataRecordFile class is not thread safe
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
	///\param[in] headersize_ size of the file header in bytes (0 for no header is default)
	///\param[in] headerstruct header structure in case of open for write
	///\return true, if success, false on failure (see error() for the system error code ~ errno)
	bool open( const std::string& filename, const Mode& mode_, unsigned int recordsize_, unsigned int headersize_=0, const void* headerstruct=0);

	///\brief Read one record of the file
	///\param[in] fpos index of the record to read (file position is the index multiplied with the record size)
	///\return pointer to buffer read if success, (0) NULL on failure (see error() for the system error code ~ errno)
	const void* read( std::size_t fpos);

	///\brief Read the record at the current file position of the file
	///\return pointer to buffer read if success, (0) NULL on failure (see error() for the system error code ~ errno)
	const void* read();

	///\brief Get a pointer to the header of of the file
	///\return pointer to the header, a structure of a size specified as last argument of DataRecordFile::open(..)
	const void* header();
	
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
	unsigned int m_headersize;	///< size of a record in bytes
	std::size_t m_recordindex;	///< current file position
	void* m_recbuf;
	void* m_hdrbuf;

private:
	DataRecordFile( const DataRecordFile&){}				///< non copyable
	DataRecordFile& operator=( const DataRecordFile&){return *this;}	///< non copyable
};
#endif

}//namespace
#endif

