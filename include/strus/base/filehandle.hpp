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

namespace strus
{

#if defined _WIN32 || defined __CYGWIN__
#error File and socket handles not implemented for Windows in Strus
#else
typedef int FileHandle;
typedef int SocketHandle;
#endif

}//namespace
#endif
