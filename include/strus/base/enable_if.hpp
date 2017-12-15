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

#if __cplusplus >= 201103L
#include <type_traits>
namespace strus {
template< bool B, class T = void >
struct enable_if :public std::enable_if<B,T> {};
}//namespace

#else

#include <boost/utility.hpp>
#include <boost/type_traits.hpp>

namespace strus {
template< bool B, class T = void >
struct enable_if :public std::enable_if<B,T>{};
}//namespace

#endif

#endif

