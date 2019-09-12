/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Basic portable file/socket handle and related system functions
/// \file "filehandle.hpp"
#ifndef _STRUS_BASE_FILE_HANDLE_HPP_INCLUDED
#define _STRUS_BASE_FILE_HANDLE_HPP_INCLUDED
#include <algorithm>
#include <utility>
#include <string>

namespace strus
{

#if defined _WIN32 || defined __CYGWIN__
#error File and socket handles not implemented for Windows in Strus
#else
typedef int FileHandle;
typedef int SocketHandle;
#endif

/// \brief Class for redirecting a file output to buffer
class WriteBufferHandle
{
public:
	/// \brief Constructor
	WriteBufferHandle();
	/// \brief Destructor
	~WriteBufferHandle();

	/// \brief Get the errno code in case of an error occurred, else 0
	/// \return the errno code
	int error() const;

	/// \brief Get the file handle provided for writing to the buffer
	/// \return the file handle
	FileHandle fileHandle() const;

	/// \brief Get a pointer to a FILE to write to
	/// \return the file handle as C stream handle
	/// \remark Do not use fileHandle anymore when using the stream
	FILE* getCStreamHandle();

	/// \brief Fetch the content of the buffer
	/// \return the buffer content
	/// \note buffer is emptied with this call
	std::string fetchContent();

	/// \brief Terminate listener
	void done();

private:
	struct Data;
	Data* m_impl;
};

}//namespace
#endif

