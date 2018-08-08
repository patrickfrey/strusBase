/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus base file locator library creating the file locator interface
/// \file filelocator.hpp
#ifndef _STRUS_BASE_FILELOCATOR_LIB_HPP_INCLUDED
#define _STRUS_BASE_FILELOCATOR_LIB_HPP_INCLUDED
#include "strus/fileLocatorInterface.hpp"
/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Create a file locator interface
/// \param[in] errorhnd error buffer interface
/// \return the file locator (with ownership)
FileLocatorInterface* createFileLocator_std( ErrorBufferInterface* errorhnd);

}//namespace
#endif

