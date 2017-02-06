/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Helper methods for reading/writing numeric values with platform independent byteorder
/// \file hton.hpp
#ifndef _STRUS_BASE_HTON_HPP_INCLUDED
#define _STRUS_BASE_HTON_HPP_INCLUDED
#include "strus/base/stdint.h"
#include <arpa/inet.h>

namespace strus
{

template <typename Scalar>
class ByteOrder
{
	typedef void net_value_type;
};

template <>
class ByteOrder<uint8_t>
{
public:
	typedef uint8_t net_value_type;

	static uint8_t hton( const uint8_t& value)	{return value;}
	static uint8_t ntoh( const uint8_t& value)	{return value;}
};

template <>
class ByteOrder<int8_t>
{
public:
	typedef int8_t net_value_type;

	static int8_t hton( const int8_t& value)	{return value;}
	static int8_t ntoh( const int8_t& value)	{return value;}
};

template <>
class ByteOrder<uint16_t>
{
public:
	typedef uint16_t net_value_type;

	static uint16_t hton( const uint16_t& value)	{return htons(value);}
	static uint16_t ntoh( const uint16_t& value)	{return ntohs(value);}
};

template <>
class ByteOrder<int16_t>
{
public:
	typedef int16_t net_value_type;

	static int16_t hton( const int16_t& value)	{return htons(value);}
	static int16_t ntoh( const int16_t& value)	{return ntohs(value);}
};

template <>
class ByteOrder<uint32_t>
{
public:
	typedef uint32_t net_value_type;

	static uint32_t hton( const uint32_t& value)	{return htonl(value);}
	static uint32_t ntoh( const uint32_t& value)	{return ntohl(value);}
};

template <>
class ByteOrder<int32_t>
{
public:
	typedef int32_t net_value_type;

	static int32_t hton( const int32_t& value)	{return htonl(value);}
	static int32_t ntoh( const int32_t& value)	{return ntohl(value);}
};

template <>
class ByteOrder<uint64_t>
{
public:
	typedef uint64_t net_value_type;

	static uint64_t hton( const uint64_t& value)
	{
		union
		{
			uint32_t p[2];
			uint64_t v;
		} val;
		val.p[0] = htonl( value >> 32);
		val.p[1] = htonl( value & 0xffFFffFF);
		return val.v;
	}
	static uint64_t ntoh( const uint64_t& value)
	{
		union
		{
			uint32_t p[2];
			uint64_t v;
		} val;
		val.v = value;
		val.p[0] = ntohl( val.p[0]);
		val.p[1] = ntohl( val.p[1]);
		uint64_t rt = val.p[0];
		rt <<= 32;
		rt |= val.p[1];
		return rt;
	}
};

template <>
class ByteOrder<int64_t>
{
public:
	typedef int64_t net_value_type;

	static int64_t hton( const int64_t& value)
	{
		return (int64_t)ByteOrder<uint64_t>::hton((uint64_t)value);
	}
	static int64_t ntoh( const int64_t& value)
	{
		return (int64_t)ByteOrder<uint64_t>::ntoh((uint64_t)value);
	}
};

typedef uint32_t float_net_t;
typedef uint64_t double_net_t;

template <>
class ByteOrder<float>
{
public:
	typedef float_net_t net_value_type;

	static net_value_type hton( const float& value)
	{
		union
		{
			net_value_type out;
			float in;
		} val;
		val.in = value;
		return ByteOrder<net_value_type>::hton( val.out);
	}
	static float ntoh( const net_value_type& value)
	{
		union
		{
			float out;
			net_value_type in;
		} val;
		val.in = ByteOrder<net_value_type>::ntoh( value);
		return val.out;
	}
};

template <>
class ByteOrder<double>
{
public:
	typedef double_net_t net_value_type;

	static net_value_type hton( const double& value)
	{
		union
		{
			net_value_type out;
			double in;
		} val;
		val.in = value;
		return ByteOrder<net_value_type>::hton( val.out);
	}
	static double ntoh( const net_value_type& value)
	{
		union
		{
			double out;
			net_value_type in;
		} val;
		val.in = ByteOrder<net_value_type>::ntoh( value);
		return val.out;
	}
};

} //namespace
#endif

