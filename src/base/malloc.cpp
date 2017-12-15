/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some malloc wrappers
#include "strus/base/malloc.hpp"
#include "strus/base/dll_tags.hpp"
#include <unistd.h>
#include <stdlib.h>

using namespace strus;

DLL_PUBLIC void strus::aligned_free( void *ptr)
{
#if (defined _MSC_VER)
	return _aligned_free( ptr);
#else
	free( ptr);
#endif
}

DLL_PUBLIC void* strus::aligned_malloc( std::size_t size, std::size_t alignment)
{
#if (defined _MSC_VER)
	return _aligned_malloc( size, alignment);
#else
	void* rt = 0;
	return (0==posix_memalign( &rt, alignment, size))?rt:0;
#endif
}


