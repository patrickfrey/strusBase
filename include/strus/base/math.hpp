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
#include <limits>
#include <cmath>
#include <cstdlib>

namespace strus {

struct Math
{
	static inline double sqrt( double xx)			{return std::sqrt( xx);}
	static inline float sqrt( float xx)			{return std::sqrt( xx);}

	static inline double floor( double xx)			{return std::floor( xx);}
	static inline double pow( double bs, double pw)		{return std::pow( bs, pw);}
	static inline double log( double xx)			{return std::log( xx);}
	static inline double log10( double xx)			{return std::log10( xx);}

	static inline double ceil( double xx)			{return std::ceil( xx);}
	static inline double exp( double xx)			{return std::exp( xx);}

	static inline double fmod( double xx, double yy)	{return std::fmod( xx,yy);}

	static inline double sin( double xx)			{return std::sin( xx);}
	static inline double cos( double xx)			{return std::cos( xx);}
	static inline double tan( double xx)			{return std::tan( xx);}

	static inline double sinh( double xx)			{return std::sinh( xx);}
	static inline double cosh( double xx)			{return std::cosh( xx);}
	static inline double tanh( double xx)			{return std::tanh( xx);}

	static inline double asin( double xx)			{return std::asin( xx);}
	static inline double acos( double xx)			{return std::acos( xx);}
	static inline double atan( double xx)			{return std::atan( xx);}

	static inline bool isnan( double xx)			{return std::isnan( xx);}
	static inline bool isnan( float xx)			{return std::isnan( xx);}

	static inline double abs( double xx)			{return std::abs( xx);}
	static inline float abs( float xx)			{return std::abs( xx);}

	static inline int abs( int xx)				{return std::abs( xx);}
	static inline long int abs( long int xx)		{return std::labs( xx);}

	static inline bool isequal( double xx, double yy, double tolerated_error = std::numeric_limits<double>::epsilon())
		{return abs( xx-yy) <= tolerated_error;}
	static inline bool isequal( float xx, float yy, float tolerated_error = std::numeric_limits<float>::epsilon())
		{return abs( xx-yy) <= tolerated_error;}
};

} //namespace
#endif

