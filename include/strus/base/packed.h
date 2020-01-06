/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* \brief Definition of integer types
*/
#ifndef _STRUS_BASE_PACKED_H_INCLUDED
#define _STRUS_BASE_PACKED_H_INCLUDED

#if defined( _MSC_VER )
#define PACKED_STRUCT( DECL)  __pragma( pack(push, 1) ) struct DECL __pragma( pack(pop))
#elif defined( __GNUC__ ) || defined( __clang__ )
#define PACKED_STRUCT( DECL)  struct __attribute__((__packed__)) DECL
#endif

#endif

