/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Definition of strus::shared_ptr
#ifndef _STRUS_BASE_SHARED_PTR_HPP_INCLUDED
#define _STRUS_BASE_SHARED_PTR_HPP_INCLUDED

#if __cplusplus >= 201103L
#define STRUS_USE_STD_SHARED_PTR
#else
#undef STRUS_USE_STD_SHARED_PTR
#endif

#if defined STRUS_USE_STD_SHARED_PTR
#include <memory>

namespace strus {

template <class X>
class shared_ptr
	:public std::shared_ptr<X>
{
public:
	shared_ptr( X* ptr)
		:std::shared_ptr<X>(ptr){}
	shared_ptr( const shared_ptr& o)
		:std::shared_ptr<X>(o){}
	shared_ptr()
		:std::shared_ptr<X>(){}
};
} //namespace

#else //STRUS_USE_STD_SHARED_PTR

#include <boost/shared_ptr.hpp>
namespace strus {

template <class X>
class shared_ptr
	:public boost::shared_ptr<X>
{
public:
	shared_ptr( X* ptr)
		:boost::shared_ptr<X>(ptr){}
	shared_ptr( const shared_ptr& o)
		:boost::shared_ptr<X>(o){}
	shared_ptr()
		:boost::shared_ptr<X>(){}
};
} //namespace

#endif //STRUS_USE_STD_SHARED_PTR

#endif

