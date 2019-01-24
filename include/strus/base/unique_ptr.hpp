/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Definition of strus::unique_ptr as wrapper to std::unique_ptr or boost::unique_ptr depeding on availability in the C++ standard library
#ifndef _STRUS_BASE_UNIQUE_PTR_HPP_INCLUDED
#define _STRUS_BASE_UNIQUE_PTR_HPP_INCLUDED

#if __cplusplus >= 201103L
#include <memory>

namespace strus {
template <typename T>
class unique_ptr : public std::unique_ptr<T>
{
public:
	explicit unique_ptr( T* p = 0 ) :std::unique_ptr<T>(p) {}
};
} //namespace
#else
#if BOOST_VERSION < 105800
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#define BOOST_UNIQUE_PTR_TEMPLATE boost::movelib::unique_ptr
#else
#include <boost/move/unique_ptr.hpp>
#define BOOST_UNIQUE_PTR_TEMPLATE boost::unique_ptr
#endif

namespace strus {
template <typename T>
struct DefaultDeleter
{
	void operator()(T* ptr) const	{delete ptr;}
	template <class U>
	void operator()(U* ptr) const	{delete[] ptr;}
};

template <typename T>
class unique_ptr
	:public BOOST_UNIQUE_PTR_TEMPLATE<T,DefaultDeleter<T> >
{
public:
	explicit unique_ptr( T* p = 0 ) :BOOST_UNIQUE_PTR_TEMPLATE<T,DefaultDeleter<T> >(p) {}
};
} //namespace
#endif
#endif


