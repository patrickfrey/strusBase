/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/reference.hpp"
#include "strus/lib/error.hpp"
#include "strus/base/pseudoRandom.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>
#include <new>
#include <iostream>
#include <cstring>

using namespace strus;

static strus::PseudoRandom g_random;

static void printUsage()
{
	std::cerr << "Usage: testReference [<size> [<nofops]]" << std::endl;
	std::cerr << "<size>     :size of array tested (default 100)" << std::endl;
	std::cerr << "<nofops>   :number of operations performed (default 100000)" << std::endl;
}

enum Op {
	OpNew,
	OpAssign,
	OpReset,
	OpRelease
};
enum {NofOps=(int)OpAssign+1};

static void checkAll( const strus::Reference<int>* ar, int arraySize)
{
	int ai = 0, ae = arraySize;
	for (; ai<ae; ++ai)
	{
		ar[ ai].check();
	}
}

static void testReference( int arraySize, int nofOps)
{
	strus::Reference<int>* ar = new strus::Reference<int>[ arraySize];
	int oi = 0, oe = nofOps;
	int aidx, oidx;

	for (; oi<oe; ++oi)
	{
		if (g_random.get( 0, 100) == 1)
		{
			checkAll( ar, arraySize);
		}
		Op op = (Op)g_random.get( 0, NofOps);
		switch (op)
		{
			case OpNew:
			{
				aidx = g_random.get( 0, arraySize);
				ar[ aidx] = strus::Reference<int>( new int( aidx));
				ar[ aidx].check();
				break;
			}
			case OpAssign:
			{
				aidx = g_random.get( 0, arraySize);
				oidx = g_random.get( 0, arraySize);
				ar[ aidx] = ar[ oidx];
				ar[ aidx].check();
				ar[ oidx].check();
				break;
			}
			case OpReset:
			{
				aidx = g_random.get( 0, arraySize);
				ar[ aidx].reset( new int( aidx));
				ar[ aidx].check();
				break;
			}
			case OpRelease:
			{
				aidx = g_random.get( 0, arraySize);
				if (ar[ aidx].use_count() == 1) delete ar[ aidx].release();
				ar[ aidx].check();
				break;
			}
				
		}
	}
	checkAll( ar, arraySize);
	delete [] ar;
}

int main( int argc, const char** argv)
{
	try
	{
		int arraySize = 200;
		int nofOps = 500000;

		if (argc > 1 && 0==std::strcmp( argv[1], "-h"))
		{
			printUsage();
			return 0;
		}
		if (argc > 1)
		{
			arraySize = atoi( argv[ 1]);
			if (arraySize == 0)
			{
				printUsage();
				throw std::runtime_error("parameter <arraysize> must be a positive integer");
			}
		}
		if (argc > 2)
		{
			nofOps = atoi( argv[ 2]);
			if (nofOps == 0)
			{
				printUsage();
				throw std::runtime_error("parameter <nofops> must be a positive integer");
			}
		}
		if (argc > 3)
		{
			printUsage();
			throw std::runtime_error("too many arguments");
		}
		std::cerr << "executing referece test" << std::endl;
		testReference( arraySize, nofOps);
		std::cerr << "OK" << std::endl;
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


