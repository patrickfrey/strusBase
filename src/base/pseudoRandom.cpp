/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Fast generator for series of pseudo random numbers where the security is not an issue and the quality of the distribution has not to be guaranteed
/// \note Mainly used for tests with a seed depending on the current day, so that repetition on the same date produce identical series
#include "strus/base/pseudoRandom.hpp"
#include "strus/base/dll_tags.hpp"
#include <ctime>
#include <cstdlib>
#include <cstdarg>
#include <stdexcept>
#include <algorithm>
#include "strus/base/stdint.h"

using namespace strus;

/// \brief Pseudo random generator 
enum {KnuthIntegerHashFactor=2654435761U};

static inline uint32_t uint32_hash( uint32_t a)
{
	a += ~(a << 15);
	a ^=  (a >> 10);
	a +=  (a << 3);
	a ^=  (a >> 6);
	a += ~(a << 11);
	a ^=  (a >> 16);
	return a;
}

static unsigned int generateSeed()
{
	time_t nowtime;
	struct tm* now;

	::time( &nowtime);
	now = ::localtime( &nowtime);

	return (now->tm_year+1900)*10000 + (now->tm_mon+1)*100 + now->tm_mday;
}

DLL_PUBLIC PseudoRandom::PseudoRandom()
{
	init( generateSeed());
}

DLL_PUBLIC PseudoRandom::PseudoRandom( int seed_)
{
	init( seed_);
}

DLL_PUBLIC void PseudoRandom::init( int seed_)
{
	m_seed = seed_;
	m_value = uint32_hash( seed_);
	m_incr = m_value * KnuthIntegerHashFactor;
}

DLL_PUBLIC int PseudoRandom::seed() const
{
	return m_seed;
}

DLL_PUBLIC int PseudoRandom::get( int min_, int max_)
{
	if (min_ >= max_)
	{
		if (min_ == max_) return min_;
		std::swap( min_, max_);
	}
	m_value = uint32_hash( m_value + 1 + m_incr++);
	unsigned int iv = max_ - min_;
	return (int)(m_value % iv) + min_;
}

DLL_PUBLIC int PseudoRandom::select( unsigned int psize, int firstValue, ...)
{
	va_list ap;
	if (psize == 0) return firstValue;
	int pidx = get( 0, psize);
	va_start( ap, firstValue);
	int rt = firstValue;
	for (int ii = 0; ii < pidx; ii++)
	{
		rt = va_arg( ap, int);
	}
	va_end(ap);
	return rt;
}

