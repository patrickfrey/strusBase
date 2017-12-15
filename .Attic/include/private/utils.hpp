/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some utility functions that are centralised to control dependencies to boost
#ifndef _STRUS_UTILS_HPP_INCLUDED
#define _STRUS_UTILS_HPP_INCLUDED
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <stdint.h>			///... boost atomic needs this
#include <boost/atomic/atomic.hpp>

namespace strus {
namespace utils {

std::string tolower( const char* val);
std::string tolower( const std::string& val);
std::string trim( const std::string& val);
bool caseInsensitiveEquals( const std::string& val1, const std::string& val2);
bool caseInsensitiveStartsWith( const std::string& val, const std::string& prefix);
int toint( const std::string& val);
std::string tostring( int val);

void aligned_free( void *ptr);
void* aligned_malloc( std::size_t size, std::size_t alignment);

class AtomicFlag
	:public boost::atomic<bool>
{
public:
	///\brief Constructor
	AtomicFlag( bool initialValue_=false)
		:boost::atomic<bool>(initialValue_)
	{}

	///\brief Set the flag, if the new value changes the current value.
	///\return true on success
	bool set( bool val)
	{
		bool prev_val = !val;
		return boost::atomic<bool>::compare_exchange_strong( prev_val, val, boost::memory_order_acquire);
	}

	///\brief Evaluate the current value
	///\return the current value
	bool test()
	{
		return boost::atomic<bool>::load( boost::memory_order_acquire);
	}
};

class ThreadId
{
public:
	typedef boost::thread::id Type;
	static Type get()
	{
		return boost::this_thread::get_id();
	}
};

#define STRUS_STATIC_ASSERT( cond ) BOOST_STATIC_ASSERT((cond))
#define CACHELINE_SIZE 64

}} //namespace
#endif


