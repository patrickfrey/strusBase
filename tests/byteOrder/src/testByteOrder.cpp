/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/hton.hpp"
#include "strus/base/pseudoRandom.hpp"
#include "strus/base/stdint.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <limits>
#include <ctime>
#include <cstdlib>

static strus::PseudoRandom g_random;

using namespace strus;

#define KNUTH_HASH 2654435761U

static int random_int()
{
	return g_random.get( 0, std::numeric_limits<int>::max());
}

template <typename ValueType>
ValueType randomValue()
{
	return random_int() * KNUTH_HASH - random_int() * KNUTH_HASH;
}

template <>
double randomValue<double>()
{
	return ((double)random_int() * KNUTH_HASH) / ((double)random_int() * KNUTH_HASH)
		- ((double)random_int() * KNUTH_HASH) / ((double)random_int() * KNUTH_HASH);
}

template <>
float randomValue<float>()
{
	return (float)(random_int() * KNUTH_HASH) / (float)(random_int() * KNUTH_HASH);
}

template <typename ValueType>
const char* typeName() {return "<unknown>";}

template <>
const char* typeName<double>() {return "double";}
template <>
const char* typeName<float>() {return "float";}
template <>
const char* typeName<uint32_t>() {return "uint32";}
template <>
const char* typeName<int32_t>() {return "int32";}
template <>
const char* typeName<uint16_t>() {return "uint16";}
template <>
const char* typeName<int16_t>() {return "int16";}
template <>
const char* typeName<uint8_t>() {return "uint8";}
template <>
const char* typeName<int8_t>() {return "int8";}


static void print_value_seq( const void* sq, unsigned int sqlen)
{
	static const char* HEX = "0123456789ABCDEF";
	unsigned char const* si = (const unsigned char*) sq;
	unsigned const char* se = (const unsigned char*) sq + sqlen;
	for (; si != se; ++si)
	{
		unsigned char lo = *si % 16, hi = *si / 16;
		char buf[ 4];
		buf[ 0] = ' ';
		buf[ 1] = HEX[hi];
		buf[ 2] = HEX[lo];
		buf[ 3] = '\0';
		std::cout << buf;
	}
	std::cout << " |";
}

template <typename ValueType>
static void byteOrderTest( unsigned int times)
{
	for (unsigned int ti=0; ti<times; ++ti)
	{
		ValueType val = randomValue<ValueType>();
		typename ByteOrder<ValueType>::net_value_type val_n = ByteOrder<ValueType>::hton( val);
		print_value_seq( &val, sizeof(val));
		std::cout << " ==>";
		print_value_seq( &val_n, sizeof(val_n));
		if (ByteOrder<ValueType>::ntoh( val_n) != val)
		{
			std::cout << " ERR" << std::endl;
			throw std::runtime_error( std::string("byte order test failed for type ") + typeName<ValueType>());
		}
		std::cout << " OK" << std::endl;
	}
}

int main( int, const char**)
{
	try
	{
		std::cerr << "executing test byteOrderTest with " << typeName< double>() << std::endl;
		byteOrderTest< double >( 10);

		std::cerr << "executing test byteOrderTest with " << typeName< float>() << std::endl;
		byteOrderTest< float >( 10);

		std::cerr << "executing test byteOrderTest with " << typeName< uint32_t>() << std::endl;
		byteOrderTest< uint32_t >( 10);

		std::cerr << "executing test byteOrderTest with " << typeName< int32_t>() << std::endl;
		byteOrderTest< int32_t >( 10);

		std::cerr << "executing test byteOrderTest with " << typeName< uint16_t>() << std::endl;
		byteOrderTest< uint16_t >( 10);

		std::cerr << "executing test byteOrderTest with " << typeName< int16_t>() << std::endl;
		byteOrderTest< int16_t >( 10);

		std::cerr << "executing test byteOrderTest with " << typeName< uint8_t>() << std::endl;
		byteOrderTest< uint8_t >( 10);

		std::cerr << "executing test byteOrderTest with " << typeName< int8_t>() << std::endl;
		byteOrderTest< int8_t >( 10);
		return 0;
	}
	catch (const std::bad_alloc& err)
	{
		std::cerr << "ERROR " << err.what() << std::endl;
	}
	catch (const std::exception& err)
	{
		std::cerr << "ERROR " << err.what() << std::endl;
	}
	return -1;
}


