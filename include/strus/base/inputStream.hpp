/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_BASE_INPUT_STREAM_HPP_INCLUDED
#define _STRUS_BASE_INPUT_STREAM_HPP_INCLUDED
#include "private/utils.hpp"
#include <string>
#include <fstream>
#include <cstdio>

namespace strus {

/// \class InputStream
/// \brief Abstraction of input stream
class InputStream
{
public:
	/// \brief Constructor
	/// \param[in] docpath path to file to read or "-" for stdin
	explicit InputStream( const std::string& docpath);

	/// \brief Destructor
	~InputStream();

	/// \brief Read some data
	/// \param[in,out] buf where to write to
	/// \param[in] bufsize allocation size of 'buf' (capacity)
	/// \return the number of bytes read, 0 in case of EOF or an error occurred
	std::size_t read( char* buf, std::size_t bufsize);

	/// \brief Read a line
	/// \param[in,out] buf where to write to
	/// \param[in] bufsize allocation size of 'buf' (capacity) 
	/// \return pointer to the line read or NULL in case of no line to read left
	const char* readLine( char* buf, std::size_t bufsize, bool failOnNoLine=false);

	/// \brief Read some data and keep it in a buffer for the next read
	/// \param[in,out] buf where to write to
	/// \param[in] bufsize allocation size of 'buf' (capacity) 
	/// \return the number of bytes read or 0 in case of EOF or an error occurred
	std::size_t readAhead( char* buf, std::size_t bufsize);

	/// \brief Get the last error (system errno) occurred
	/// \return the last error
	unsigned int error() const;

	/// \brief Check if there is more data to read
	bool eof();

private:
	FILE* m_fh;
	std::string m_docpath;
	std::ifstream m_stream;
	std::string m_buffer;
	std::size_t m_bufferidx;
	unsigned int m_errno;
};

}
#endif


