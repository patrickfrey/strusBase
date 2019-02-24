/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/bitset.hpp"
#include "strus/base/dynamic_bitset.hpp"
#include "strus/base/string_format.hpp"
#include "strus/base/pseudoRandom.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <limits>
#include <cstdlib>
#include <set>

#undef STRUS_LOWLEVEL_DEBUG

static int g_nof_errors = 0;
static int g_max_nof_errors = 1;
static strus::PseudoRandom g_random;

static void set_remove( std::set<int>& eset, int elem)
{
	std::set<int> new_eset;
	std::set<int>::const_iterator ei = eset.begin(), ee = eset.end();
	for (; ei != ee; ++ei)
	{
		if (*ei < elem)
		{
			new_eset.insert( *ei);
		}
		else if (*ei > elem)
		{
			new_eset.insert( *ei-1);
		}
	}
	eset = new_eset;
}

static void set_insert( std::set<int>& eset, int elem, int maxvalue)
{
	std::set<int> new_eset;
	std::set<int>::const_iterator ei = eset.begin(), ee = eset.end();
	for (; ei != ee; ++ei)
	{
		if (*ei < elem)
		{
			new_eset.insert( *ei);
		}
		else if (*ei >= elem)
		{
			if (*ei+1 < maxvalue)
			{
				new_eset.insert( *ei+1);
			}
		}
	}
	if (elem < maxvalue)
	{
		new_eset.insert( elem);
	}
	eset = new_eset;
}

static void testDynamicBitSet( int times, int maximum, int nofElements)
{
	if (nofElements >= maximum) throw std::runtime_error("potential endless loop in test");

	for (int ti=0; ti<times; ++ti)
	{
		strus::dynamic_bitset testset( maximum);
		std::set<int> eset;
		for (int ei=0; ei<nofElements; ++ei)
		{
			int elem = g_random.get( 0, maximum);
			eset.insert( elem);
			testset.set( elem, true);
		}
		std::set<int>::const_iterator si = eset.begin(), se = eset.end();
		for (; si != se; ++si)
		{
			if (!testset.test( *si))
			{
				std::cerr << "element " << *si << " inserted not found in dynamic bitset" << std::endl;
				if (++g_nof_errors >= g_max_nof_errors) throw std::runtime_error( "dynamic bitset test failed");
			}
		}
		for (int ei=0; ei<nofElements; ++ei)
		{
			int elem = g_random.get( 0, maximum);
			if (eset.find( elem) != eset.end())
			{
				--ei; continue;
			}
			if (testset.test( elem))
			{
				std::cerr << "element " << elem << " not inserted but found in dynamic bitset" << std::endl;
				if (++g_nof_errors >= g_max_nof_errors) throw std::runtime_error( "dynamic bitset test failed");
			}
		}
	}
	std::cerr << "executed testDynamicBitSet( " << times << ", " << maximum << ", " << nofElements << ")" << std::endl;
}

template<int NN>
static void checkElements( const strus::bitset<NN>& testset, const std::set<int>& eset)
{
#ifdef STRUS_LOWLEVEL_DEBUG
	std::cerr << "elements in testset = {";
	std::vector<int> elist = testset.elements();
	std::vector<int>::const_iterator li = elist.begin(), le = elist.end();
	for (int lidx=0; li != le; ++li,++lidx)
	{
		if (lidx) std::cerr << ",";
		std::cerr << *li;
	}
	std::cerr << "}" << std::endl;
	std::cerr << "elements expected = {";
	std::set<int>::const_iterator qi = eset.begin(), qe = eset.end();
	for (int qidx=0; qi != qe; ++qi,++qidx)
	{
		if (qidx) std::cerr << ",";
		std::cerr << *qi;
	}
	std::cerr << "}" << std::endl;
#endif
	std::set<int>::const_iterator si = eset.begin(), se = eset.end();
	int gi = testset.first();
	for (; gi >= 0 && si != se; ++si,gi=testset.next(gi))
	{
		if (gi != *si)
		{
			std::cerr << "bitset iterator does not return elements expected: " << gi << " != " << *si << std::endl;
			if (++g_nof_errors >= g_max_nof_errors) throw std::runtime_error( "fixed size bitset test failed");
		}
	}
	if (gi != -1 || si != se)
	{
		if (gi == -1)
		{
			std::cerr << "bitset iterator does not return elements expected: EOF != " << *si << std::endl;
		}
		else
		{
			std::cerr << "bitset iterator does not return elements expected: " << gi << " != EOF" << std::endl;
		}
		if (++g_nof_errors >= g_max_nof_errors) throw std::runtime_error( "fixed size bitset test failed");
	}
}

template<int NN>
static void testBitSet( int times, int nofElements)
{
	std::cerr << "execute testBitSet<" << NN << ">( " << times << ", " << nofElements << ")" << std::endl;
	if (nofElements >= NN) throw std::runtime_error("potential endless loop in test");

	for (int ti=0; ti<times; ++ti)
	{
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << "start [" << ti << "]" << std::endl;
#endif
		strus::bitset<NN> testset;
		std::set<int> eset;
		for (int ei=0; ei<nofElements; ++ei)
		{
			int elem = g_random.get( 0, NN);
			switch (g_random.get( 0, 5))
			{
				case 0:
#ifdef STRUS_LOWLEVEL_DEBUG
					std::cerr << "op insert " << elem << std::endl;
#endif
					set_insert( eset, elem, NN);
					if (!testset.insert( elem, true)) throw std::runtime_error("insert failed in bitset");
					break;
				case 1:
#ifdef STRUS_LOWLEVEL_DEBUG
					std::cerr << "op remove " << elem << std::endl;
#endif
					set_remove( eset, elem);
					if (!testset.remove( elem)) throw std::runtime_error("remove failed in bitset");
					break;
				default:
#ifdef STRUS_LOWLEVEL_DEBUG
					std::cerr << "op set " << elem << std::endl;
#endif
					eset.insert( elem);
					testset.set( elem, true);
					break;
			}
			checkElements( testset, eset);
		}
		std::set<int>::const_iterator si = eset.begin(), se = eset.end();
		for (; si != se; ++si)
		{
			if (!testset.test( *si))
			{
				std::cerr << "element " << *si << " inserted not found in fixed size test bitset" << std::endl;
				if (++g_nof_errors >= g_max_nof_errors) throw std::runtime_error( "fixed size bitset test failed");
			}
		}
		for (int ei=0; ei<nofElements; ++ei)
		{
			int elem = g_random.get( 0, NN);
			if (eset.find( elem) != eset.end())
			{
				--ei; continue;
			}
			if (testset.test( elem))
			{
				std::cerr << "element " << elem << " not inserted but found in fixed size test bitset" << std::endl;
				if (++g_nof_errors >= g_max_nof_errors) throw std::runtime_error( "fixed size bitset test failed");
			}
		}
		checkElements( testset, eset);
	}
}


#define NOF_TESTS 1000

int main( int, const char**)
{
	try
	{
		testBitSet<8>( 20, 1);
		testBitSet<16>( 20, 1);
		testBitSet<32>( 30, 1);
		testBitSet<32>( 30, 10);
		testBitSet<32>( 30, 20);
		testBitSet<32>( 30, 30);
		testBitSet<64>( 30, 1);
		testBitSet<64>( 30, 10);
		testBitSet<64>( 30, 20);
		testBitSet<64>( 30, 30);
		testBitSet<64>( 30, 50);
		testBitSet<128>( 30, 1);
		testBitSet<128>( 30, 10);
		testBitSet<128>( 30, 20);
		testBitSet<128>( 30, 30);
		testBitSet<128>( 30, 50);
		testBitSet<128>( 30, 100);
		testBitSet<256>( 30, 10);
		testBitSet<256>( 30, 20);
		testBitSet<256>( 30, 30);
		testBitSet<256>( 30, 50);
		testBitSet<256>( 30, 100);
		testBitSet<256>( 30, 200);
		testBitSet<512>( 30, 1);
		testBitSet<512>( 30, 10);
		testBitSet<512>( 30, 20);
		testBitSet<512>( 30, 30);
		testBitSet<512>( 30, 50);
		testBitSet<512>( 30, 100);
		testBitSet<512>( 30, 200);

		testDynamicBitSet( 30, 10, 5);
		testDynamicBitSet( 30, 100, 50);
		testDynamicBitSet( 30, 1000, 500);
		testDynamicBitSet( 30, 1000, 5);
		testDynamicBitSet( 20, 10000, 5000);
		testDynamicBitSet( 20, 10000, 50);
		testDynamicBitSet( 5, 100000, 50000);
		testDynamicBitSet( 5, 100000, 500);
		testDynamicBitSet( 5, 100000, 50);
		testDynamicBitSet( 2, 1000000, 500000);
		testDynamicBitSet( 2, 1000000, 5000);
		testDynamicBitSet( 2, 1000000, 500);
		testDynamicBitSet( 2, 1000000, 50);

		if (g_nof_errors > 0)
		{
			std::cerr << "ERROR test failed with " << g_nof_errors << " errors" << std::endl;
			return -1;
		}
		else
		{
			std::cerr << "OK" << std::endl;
			return 0;
		}
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


