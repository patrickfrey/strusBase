/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Execution of a command appending results to string
#ifndef _STRUS_BASE_EXEC_HPP_INCLUDED
#define _STRUS_BASE_EXEC_HPP_INCLUDED
#include <string>
#include <map>

namespace strus
{

/// \brief Execution of a command appending the output to a string
/// \param[in] filename path of program to execute
/// \param[in] argv NULL terminated list of arguments to pass to program
/// \param[out] output where to append the standard output to
/// \return 0 on success, errno on failure
int execv_tostring( const char* filename, const char* const argv[], std::string& output);

/// \brief Execution of a command with some environment variables set, appending the output to a string
/// \param[in] filename path of program to execute
/// \param[in] argv NULL terminated list of arguments to pass to program
/// \param[in] env map of environment variables to set for the command execution
/// \param[out] output where to append the standard output to
/// \return 0 on success, errno on failure
int execve_tostring( const char* filename, const char* const argv[], const std::map<std::string,std::string>& env, std::string& output);

}
#endif


