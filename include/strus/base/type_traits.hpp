/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Wrapper to std or boost type traits
#ifndef _STRUS_BASE_TYPE_TRAITS_HPP_INCLUDED
#define _STRUS_BASE_TYPE_TRAITS_HPP_INCLUDED

#if __cplusplus >= 201103L
#include <type_traits>

namespace strus {

template< typename T >
struct is_arithmetic :public std::is_arithmetic<T> {};

template< typename T >
struct is_pointer :public std::is_pointer<T> {};

template< typename T, typename U >
struct is_same :public std::is_same<T,U> {};

}//namespace

#else

#include <boost/type_traits.hpp>

namespace strus {

template< typename T >
struct is_arithmetic :public boost::is_arithmetic<T> {};

template< typename T >
struct is_pointer :public boost::is_pointer<T> {};

template< typename T, typename U >
struct is_same :public boost::is_same<T,U> {};

}//namespace

#endif

#endif

