/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Wrapper to solve portability issues and problematic errors with cmath and cstdlib
#ifndef _STRUS_BASE_MATH_HPP_INCLUDED
#define _STRUS_BASE_MATH_HPP_INCLUDED
#include <cmath>
#include <cstdlib>

namespace strus {

struct Math
{
	static double sqrt( double xx)			{return std::sqrt( xx);}
	static float sqrt( float xx)			{return std::sqrt( xx);}

	static double floor( double xx)			{return std::floor( xx);}
	static double pow( double bs, double pw)	{return std::pow( bs, pw);}
	static double log( double xx)			{return std::log( xx);}
	static double log10( double xx)			{return std::log10( xx);}
	static double atan( double xx)			{return std::atan( xx);}
	static bool isnan( double xx)			{return std::isnan( xx);}
	static bool isnan( float xx)			{return std::isnan( xx);}

	static double abs( double xx)			{return std::abs( xx);}
	static float abs( float xx)			{return std::abs( xx);}

	static int abs( int xx)				{return std::abs( xx);}
	static long int abs( long int xx)		{return std::labs( xx);}
};

} //namespace
#endif

