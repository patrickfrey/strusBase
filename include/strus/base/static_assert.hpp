/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Static assertions
#ifndef _STRUS_STATIC_ASSERT_HPP_INCLUDED
#define _STRUS_STATIC_ASSERT_HPP_INCLUDED

/// PF:HACK: Bad solution, need probing of static_assert as C++ feature as for regex
#if defined __GNUC__
#define STRUS_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#undef STRUS_USE_STD_STATIC_ASSERT
#if __cplusplus >= 201103L
#if defined __clang__
#define STRUS_USE_STD_STATIC_ASSERT
#elif defined __GNUC__
#if STRUS_GCC_VERSION >= 40900
#define STRUS_USE_STD_STATIC_ASSERT
#endif // STRUS_GCC_VERSION
#endif // __clang__
#endif // __cplusplus

#ifdef STRUS_USE_STD_STATIC_ASSERT
#include <type_traits>
#define STRUS_STATIC_ASSERT( cond )	static_assert( (cond), "static assertion failed")
#else //STRUS_USE_STD_STATIC_ASSERT
#include <boost/static_assert.hpp>
#define STRUS_STATIC_ASSERT( cond ) BOOST_STATIC_ASSERT((cond))
#endif //STRUS_USE_STD_STATIC_ASSERT

#endif

