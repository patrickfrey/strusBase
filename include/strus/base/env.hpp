/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Accessing environment variables
#ifndef _STRUS_BASE_EXEC_HPP_INCLUDED
#define _STRUS_BASE_EXEC_HPP_INCLUDED
#include <string>
#include <vector>

namespace strus
{

/// \brief Read an environment variable value as string
/// \param[in] var environment variable name
/// \param[out] output where to write the value to
/// \return 0 on success, errno on failure
int getenv( const std::string& var, std::string& output);

/// \brief Read an environment variable value as array of strings, separated by a separator character
/// \param[in] var environment variable name
/// \param[in] sep separator character separating the list elements
/// \param[out] output where to append the values to
/// \return 0 on success, errno on failure
int getenv_list( const std::string& var, char sep, std::vector<std::string>& output);

/// \brief Separator character used for separating file paths in environment variables
/// \return the separator
/// \remark OS dependent 
char separatorPathList();

}
#endif


