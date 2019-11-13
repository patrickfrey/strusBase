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
	typedef void (*Deleter)( X* ptr);

public:
	shared_ptr( X* ptr)
		:std::shared_ptr<X>(ptr){}
	shared_ptr( X* ptr, Deleter deleter)
		:std::shared_ptr<X>(ptr,deleter){}
	shared_ptr( const shared_ptr& o)
		:std::shared_ptr<X>(o){}
	shared_ptr( const std::shared_ptr<X>& o)
		:std::shared_ptr<X>(o){}
	shared_ptr()
		:std::shared_ptr<X>(){}
};

template <typename Element>
strus::shared_ptr<Element> make_shared()
{
	return std::make_shared<Element>();
}

template <typename Element>
strus::shared_ptr<Element> make_shared( const Element& o)
{
	return std::make_shared<Element>( o);
}

} //namespace

#else //STRUS_USE_STD_SHARED_PTR

#include <boost/shared_ptr.hpp>
namespace strus {

template <class X>
class shared_ptr
	:public boost::shared_ptr<X>
{
public:
	typedef void (*Deleter)( X* ptr);

public:
	shared_ptr( X* ptr)
		:boost::shared_ptr<X>(ptr){}
	shared_ptr( X* ptr, Deleter deleter)
		:boost::shared_ptr<X>(ptr,deleter){}
	shared_ptr( const shared_ptr& o)
		:boost::shared_ptr<X>(o){}
	shared_ptr( const boost::shared_ptr<X>& o)
		:boost::shared_ptr<X>(o){}
	shared_ptr()
		:boost::shared_ptr<X>(){}
};

template <typename Element>
strus::shared_ptr<Element> make_shared()
{
	return boost::make_shared<Element>();
}

template <typename Element>
strus::shared_ptr<Element> make_shared( const Element& o)
{
	return boost::make_shared<Element>( o);
}

} //namespace

#endif //STRUS_USE_STD_SHARED_PTR

#endif

