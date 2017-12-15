/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some malloc wrappers
#ifndef _STRUS_MALLOC_HPP_INCLUDED
#define _STRUS_MALLOC_HPP_INCLUDED
#include <string>

namespace strus {

void aligned_free( void *ptr);
void* aligned_malloc( std::size_t size, std::size_t alignment);

}//namespace
#endif

