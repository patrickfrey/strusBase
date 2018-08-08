/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Wrapper to std::enable_if/boost::enable_if_c 
#ifndef _STRUS_BASE_ENABLE_IF_HPP_INCLUDED
#define _STRUS_BASE_ENABLE_IF_HPP_INCLUDED

/// PF:HACK: Bad solution, need probing of enable_if as C++ feature as for regex
#if defined __GNUC__
#define STRUS_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#undef STRUS_USE_STD_ENABLE_IF
#if __cplusplus >= 201103L
#if defined __clang__
#define STRUS_USE_STD_ENABLE_IF
#elif defined __GNUC__
#if STRUS_GCC_VERSION >= 40900
#define STRUS_USE_STD_ENABLE_IF
#endif // STRUS_GCC_VERSION
#endif // __clang__
#endif // __cplusplus

#if defined STRUS_USE_STD_ENABLE_IF
#include <type_traits>
namespace strus {
template< bool B, class T = void >
struct enable_if :public std::enable_if<B,T> {};
}//namespace

#else //STRUS_USE_STD_ENABLE_IF

#include <boost/utility.hpp>
#include <boost/type_traits.hpp>

namespace strus {
template< bool B, class T = void >
struct enable_if :public boost::enable_if_c<B,T>{};
}//namespace

#endif //STRUS_USE_STD_ENABLE_IF

#endif

