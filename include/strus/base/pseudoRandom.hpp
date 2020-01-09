/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_BASE_PSEUDO_RANDOM_NUMBERS_HPP_INCLUDED
#define _STRUS_BASE_PSEUDO_RANDOM_NUMBERS_HPP_INCLUDED
/// \brief Fast generator for series of pseudo random numbers where the security is not an issue and the quality of the distribution has not to be guaranteed
/// \note Mainly used for tests with a seed depending on the current day, so that repetition on the same date produce identical series
#include <ctime>
#include <cstdlib>
#include <cstdarg>
#include <stdexcept>
#include "strus/base/stdint.h"

namespace strus
{

/// \brief Pseudo random generator. Fast generator for series of pseudo random numbers where the security is not an issue and the quality of the distribution has not to be guaranteed
class PseudoRandom
{
public:
	/// \brief Constructor with seed derived from current date (granularity day)
	PseudoRandom();
	/// \brief Constructor with seed passed as argument
	/// \param[in] seed to use, generated number series deterministic for a given seed
	explicit PseudoRandom( int seed_);

	/// \brief Get the seed, this pseudo random number generator was created with
	int seed() const;

	/// \brief Get the next "random" number between two numbers
	/// \param[in] start_ minumum result
	/// \param[in] end_ upper border of range (first number not included)
	/// \return "randomly" selected integer in [start_,end_[
	int get( int start_, int end_);

	/// \brief Get the next "random" value of a specified set
	/// \param[in] psize size of candidate set
	/// \param[in] firstValue first result candidate (for ensuring that there is at least one)
	/// \param[in] ... list of other candidates to select one from (int's)
	/// \return "randomly" selected candidate
	int select( unsigned int psize, int firstValue, ...);

	/// \brief Reinitialize with a different seed
	/// \param[in] seed to use, generated number series deterministic for a given seed
	void init( int seed_);

private:
	unsigned int m_seed;
	unsigned int m_value;
	unsigned int m_incr;
};

}//namespace
#endif



