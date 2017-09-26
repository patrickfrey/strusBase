/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Definition of strus::local_ptr a substitute for std::auto_ptr used in a local scope without copying or std::unique_ptr for C++ >= 11
#ifndef _STRUS_BASE_LOCAL_PTR_HPP_INCLUDED
#define _STRUS_BASE_LOCAL_PTR_HPP_INCLUDED
#include <memory>

namespace strus {

#if __cplusplus >= 201103L
template <typename T>
class local_ptr : public std::unique_ptr<T>
{
public:
	// implement the constructors for easier use
	// in particular
	explicit local_ptr( T* p = 0 ) :std::unique_ptr<T>(p) {}
};
#else
template <typename T>
class local_ptr : public std::auto_ptr<T>
{
public:
	// implement the constructors for easier use
	// in particular
	explicit local_ptr( T* p = 0 ) :std::auto_ptr<T>(p) {}
};
#endif

} //namespace
#endif

