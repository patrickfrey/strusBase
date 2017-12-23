/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Wrapper to structures needed for atomic counters
#ifndef _STRUS_ATOMIC_HPP_INCLUDED
#define _STRUS_ATOMIC_HPP_INCLUDED

/// PF:HACK: Bad solution, need probing of atomic as C++ feature as for regex
#if defined __GNUC__
#define STRUS_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#undef STRUS_USE_STD_ATOMIC
#if __cplusplus >= 201103L
#if defined __clang__
#define STRUS_USE_STD_ATOMIC
#elif defined __GNUC__
#if STRUS_GCC_VERSION >= 40900
#define STRUS_USE_STD_ATOMIC
#endif // STRUS_GCC_VERSION
#endif // __clang__
#endif // __cplusplus

#define STRUS_CACHELINE_SIZE 64
//... TODO: Probing of this constant

#ifdef STRUS_USE_STD_ATOMIC
#include <memory>
#include <atomic>

namespace strus {

template <class X>
class atomic
	:public std::atomic<X>
{
public:
	atomic( X value)
		:std::atomic<X>(value){}
	atomic( const atomic& o)
		:std::atomic<X>(o){}
};

}//namespace

#define STRUS_MEMORY_ORDER_ATOMIC_COUNTER std::memory_order_acquire

#else //STRUS_USE_STD_ATOMIC
#include <boost/atomic.hpp>

namespace strus {

template <class X>
class atomic
	:public boost::atomic<X>
{
public:
	atomic( X value)
		:boost::atomic<X>(value){}
	atomic( const atomic& o)
		:boost::atomic<X>(o){}
};

}//namespace

#define STRUS_MEMORY_ORDER_ATOMIC_COUNTER boost::memory_order_acquire

#endif //STRUS_USE_STD_ATOMIC

namespace strus {

template <typename IntegralCounterType>
class AtomicCounter
	:public strus::atomic<IntegralCounterType>
{
public:
	///\brief Constructor
	AtomicCounter( IntegralCounterType initialValue_=0)
		:strus::atomic<IntegralCounterType>(initialValue_)
	{}

	///\brief Increment of the counter
	void increment( IntegralCounterType val = 1)
	{
		strus::atomic<IntegralCounterType>::fetch_add( val, STRUS_MEMORY_ORDER_ATOMIC_COUNTER);
	}

	///\brief Decrement of the counter
	void decrement( IntegralCounterType val = 1)
	{
		strus::atomic<IntegralCounterType>::fetch_sub( val, STRUS_MEMORY_ORDER_ATOMIC_COUNTER);
	}

	///\brief Increment of the counter
	///\return the new value of the counter after the increment operation
	IntegralCounterType allocIncrement( IntegralCounterType val = 1)
	{
		return strus::atomic<IntegralCounterType>::fetch_add( val, STRUS_MEMORY_ORDER_ATOMIC_COUNTER);
	}

	///\brief Increment of the counter
	///\return the current value of the counter
	IntegralCounterType value() const
	{
		return strus::atomic<IntegralCounterType>::load( STRUS_MEMORY_ORDER_ATOMIC_COUNTER);
	}

	///\brief Initialization of the counter
	///\param[in] val the value of the counter
	void set( const IntegralCounterType& val)
	{
		strus::atomic<IntegralCounterType>::store( val);
	}

	///\brief Compare current value with 'testval', change it to 'newval' if matches
	///\param[in] testval the value of the counter
	///\param[in] newval the value of the counter
	///\return true on success
	bool test_and_set( IntegralCounterType testval, IntegralCounterType newval)
	{
		return strus::atomic<IntegralCounterType>::compare_exchange_strong( testval, newval, STRUS_MEMORY_ORDER_ATOMIC_COUNTER, STRUS_MEMORY_ORDER_ATOMIC_COUNTER);
	}
};

class AtomicFlag
	:public strus::atomic<bool>
{
public:
	///\brief Constructor
	AtomicFlag( bool initialValue_=false)
		:strus::atomic<bool>(initialValue_)
	{}

	///\brief Set the flag, if the new value changes the current value.
	///\return true on success
	bool set( bool val)
	{
		bool prev_val = !val;
		return strus::atomic<bool>::compare_exchange_strong( prev_val, val, STRUS_MEMORY_ORDER_ATOMIC_COUNTER);
	}

	///\brief Evaluate the current value
	///\return the current value
	bool test()
	{
		return strus::atomic<bool>::load( STRUS_MEMORY_ORDER_ATOMIC_COUNTER);
	}
};

}//namespace
#endif

