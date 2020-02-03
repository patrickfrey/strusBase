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
#include "strus/base/numstring.hpp"
#include "strus/base/pseudoRandom.hpp"
#include "strus/base/minimalCover.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/localErrorBuffer.hpp"
#include "primeNumbers.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <utility>

static strus::PseudoRandom g_random;
static strus::LocalErrorBuffer g_errorbuf;

template <class TYPE>
static void shuffle( std::vector<TYPE>& vv)
{
	std::size_t vi = 0, ve = vv.size();
	for (; vi != ve; ++vi)
	{
		std::size_t xi = g_random.get( 0, vv.size());
		std::size_t yi = g_random.get( 0, vv.size());
		if (xi != yi)
		{
			std::swap( vv[ xi], vv[ yi]);
		}
	}
}

/// \brief Generate a set of all numbers divisible by a factor that are smaller than limit
static std::vector<int> generateSet( int factor, int limit)
{
	std::vector<int> rt;
	if (factor == 0) return std::vector<int>();
	for (int ii=1; ii*factor < limit; ++ii)
	{
		rt.push_back( ii*factor);
	}
	return rt;
}

static std::vector<int> joinSets( const std::vector<int>& aa, const std::vector<int>& bb)
{
	std::set<int> res;
	std::size_t ai=0, ae = g_random.get( 0,aa.size());
	for (; ai<ae; ++ai)
	{
		res.insert( aa[ g_random.get( 0,aa.size())]);
	}
	std::size_t bi=0, be = g_random.get( 0,bb.size());
	for (; bi<be; ++bi)
	{
		res.insert( bb[ g_random.get( 0,bb.size())]);
	}
	return std::vector<int>( res.begin(), res.end());
}

static bool isPrime( int num)
{
	int pi = 0, pe = NofPrimeNumbers;
	if (num <= 1) return false;

	while (pi < pe && num > g_primeNumbers[pi] * g_primeNumbers[pi])
	{
		if (num % g_primeNumbers[pi] == 0)
		{
			return false;
		}
		++pi;
	}
	return true;
}

static std::vector<int> removePrimes( const std::vector<int>& aa)
{
	std::vector<int> rt;
	std::size_t ai=0, ae = aa.size();
	for (; ai<ae; ++ai)
	{
		if (!isPrime( aa[ai]))
		{
			rt.push_back( aa[ai]);
		}
	}
	return rt;
}

static std::vector<int> getPrimeFactors( int number, bool unique)
{
	std::vector<int> rt;
	int pi = 0, pe = NofPrimeNumbers;
	if (number <= 1) return std::vector<int>();

	while (pi < pe)
	{
		while (number % g_primeNumbers[pi] == 0)
		{
			if (!unique || rt.empty() || rt.back() != g_primeNumbers[pi])
			{
				rt.push_back( g_primeNumbers[pi]);
			}
			number /= g_primeNumbers[pi];
		}
		if (number == 1) return rt;
		++pi;
	}
	throw std::runtime_error("number out of range");
}

static int compare( const std::set<int>& aa, const std::set<int>& bb)
{
	if (aa.size() == bb.size())
	{
		std::set<int>::const_iterator ai = aa.begin(), ae = aa.end();
		std::set<int>::const_iterator bi = bb.begin(), be = bb.end();
		for (; ai != ae && bi != be && *ai == *bi; ++ai,++bi){}
		if (ai == ae && bi == be)
		{
			return 0;
		}
		else return *ai - *bi;
	}
	else return aa.size() - bb.size();
}

static int compare( const std::map<int,bool>& aa, const std::map<int,bool>& bb)
{
	if (aa.size() == bb.size())
	{
		std::map<int,bool>::const_iterator ai = aa.begin(), ae = aa.end();
		std::map<int,bool>::const_iterator bi = bb.begin(), be = bb.end();
		for (; ai != ae && bi != be && *ai == *bi; ++ai,++bi){}
		if (ai == ae && bi == be)
		{
			return 0;
		}
		else if (ai->first == bi->first)
		{
			return (int)ai->second - (int)bi->second;
		} return ai->first - bi->first;
	}
	else
	{
		return aa.size() - bb.size();
	}
}

static std::vector<int> calculateSmallestPrimeNumberCoverAppriximation( const std::vector<int>& numbers)
{
	struct Solution
	{
		int count;
		std::map<int,bool> primes;
		std::set<int> numbers;

		explicit Solution( int count_)
			:count(count_),primes(),numbers(){}
		explicit Solution( const std::vector<int>& numbers_)
			:count(0),numbers(numbers_.begin(),numbers_.end())
		{
			std::vector<int>::const_iterator ni = numbers_.begin(), ne = numbers_.end();
			for (; ni != ne; ++ni)
			{
				std::vector<int> pf = getPrimeFactors( *ni, true);
				std::vector<int>::const_iterator pi = pf.begin(), pe = pf.end();
				for (; pi != pe; ++pi)
				{
					primes[ *pi] = false;
				}
			}
		}
		Solution( const Solution& o)
			:count(o.count),primes(o.primes),numbers(o.numbers){}
		Solution& operator=( const Solution& o)
			{count=o.count; primes=o.primes; numbers=o.numbers; return *this;}
#if __cplusplus >= 201103L
		Solution( Solution&& o)
			:count(o.count),primes(std::move(o.primes)),numbers(std::move(o.numbers)){}
		Solution& operator=( Solution&& o)
			{count=o.count; primes=std::move(o.primes); numbers=std::move(o.numbers); return *this;}
#endif
		bool operator < (const Solution& o) const
		{
			if (numbers.size() == o.numbers.size())
			{
				if (count == o.count)
				{
					if (primes.size() == o.primes.size())
					{
						int cmp = compare( primes, o.primes);
						if (cmp == 0)
						{
							return compare( numbers, o.numbers) < 0;
						}
						else return cmp < 0;
					}
					else return primes.size() < o.primes.size();
				}
				else return count < o.count;
			}
			else return numbers.size() < o.numbers.size();
		}

		void clearSingles()
		{
			std::map<int,bool>::iterator pi = primes.begin(), pe = primes.end();
			for (; pi != pe; ++pi)
			{
				if (numbers.find( pi->first) != numbers.end())
				{
					count += 1;
					pi->second = true;
					numbers.erase( pi->first);
				}
			}
		}

		std::vector<Solution> follow() const
		{
			std::vector<Solution> rt;
			std::map<int,bool>::const_iterator pi = primes.begin(), pe = primes.end();
			for (; pi != pe; ++pi)
			{
				std::vector<int> follow_numbers;
				bool used = false;

				std::set<int>::const_iterator ni = numbers.begin(), ne = numbers.end();
				for (; ni != ne; ++ni)
				{
					if (*ni % pi->first == 0)//divisible by this prime
					{
						used = true;
					}
					else
					{
						follow_numbers.push_back( *ni);
					}
				}
				rt.push_back( Solution( count));

				Solution& next = rt.back();
				next.primes = primes;
				if (next.primes[ pi->first] == false)
				{
					if (used)
					{
						next.count += 1;
						next.primes[ pi->first] = true;
					}
					else
					{
						next.primes.erase( pi->first);
					}
				}
				next.numbers.insert( follow_numbers.begin(), follow_numbers.end());
				next.checkCount();
			}
			return rt;
		}

		void checkCount() const
		{
			int cnt = 0;
			std::map<int,bool>::const_iterator pi = primes.begin(), pe = primes.end();
			for (; pi != pe; ++pi)
			{
				if (pi->second) ++cnt;
			}
			if (cnt != count)
			{
				throw std::runtime_error("logic error: check count failed in test data");
			}
		}

		std::vector<int> result()
		{
			std::vector<int> rt;
			std::map<int,bool>::const_iterator pi = primes.begin(), pe = primes.end();
			for (; pi != pe; ++pi)
			{
				if (pi->second) rt.push_back( pi->first);
			}
			return rt;
		}
	};
	if (numbers.empty()) return std::vector<int>();

	std::set<Solution> queue;
	Solution next( numbers);
	next.clearSingles();
	queue.insert( next);

	while (!queue.empty())
	{
		Solution top = *queue.begin();
		queue.erase( queue.begin());
		if (top.numbers.empty())
		{
			return top.result();
		}
		std::vector<Solution> follow = top.follow();
		queue.insert( follow.begin(), follow.end());
	}
	throw std::runtime_error("logic error caclulating size of cover expected");
}

static std::string integerListToString( const std::vector<int>& numbers, const char* separator)
{
	std::string rt;
	std::vector<int>::const_iterator ni = numbers.begin(), ne = numbers.end();
	for (; ni != ne; ++ni)
	{
		if (!rt.empty()) rt.append( separator);
		rt.append( strus::string_format( "%d", *ni));
	}
	return rt;
}

static void printUsage()
{
	std::cout << "Usage: testMinimalCover [-h,-V,-T <testidx>] <nof sets> <nof numbers> <nof tests>" << std::endl;
}


int main( int argc, const char** argv)
{
	try
	{
		int nofSets = 100;
		int maxNumber = 100;
		int maxPrimeNumber = g_primeNumbers[ NofPrimeNumbers-1];
		int nofTests = 1;
		int argi = 1;
		int testIdx = -1;
		float errorTolerance = 0.15; //< error tolerance 15%
		//... as we deal with approximations as result, we accept an error within this boundary

		bool verbose = false;
		for (; argi < argc; ++argi)
		{
			if (0==std::strcmp( argv[argi],"-h"))
			{
				printUsage();
				exit(0);
			}
			else if (0==std::strcmp( argv[argi],"-V"))
			{
				verbose = true;
			}
			else if (0==std::strcmp( argv[argi],"-T"))
			{
				++argi;
				testIdx = strus::numstring_conv::touint( argv[argi], std::numeric_limits<int>::max());
			}
			else if (0==std::strcmp( argv[argi],"--"))
			{
				++argi;
				break;
			}
			else if (argv[argi][0] == '-')
			{
				throw std::runtime_error( strus::string_format( "unknown option '%s'", argv[argi]));
			}
			else
			{
				break;
			}
		}
		if (argi < argc)
		{
			nofSets = strus::numstring_conv::touint( argv[argi], std::numeric_limits<int>::max());
			if (nofSets == 0)
			{
				throw std::runtime_error( strus::string_format( "number of sets (first argument) out of range: %s", argv[argi]));
			}
			++argi;
		}
		if (argi < argc)
		{
			maxNumber = strus::numstring_conv::touint( argv[argi], std::numeric_limits<int>::max());
			if (maxNumber <= 2 || maxNumber > maxPrimeNumber)
			{
				throw std::runtime_error( strus::string_format( "number of sets (first argument) out of range: %s", argv[argi]));
			}
			++argi;
		}
		if (argi < argc)
		{
			nofTests = strus::numstring_conv::touint( argv[argi], std::numeric_limits<int>::max());
			++argi;
		}
		if (argi < argc)
		{
			printUsage();
			throw std::runtime_error("too many arguments");
		}
		if (testIdx >= nofTests)
		{
			throw std::runtime_error("index of test (option -T) out of range");
		}
		if (verbose)
		{
			std::cerr << "parameter number of sets " << nofSets << std::endl;
			std::cerr << "parameter maximum element value in a set " << maxNumber << std::endl;
			std::cerr << "parameter number of tests " << nofTests << std::endl;
			if (testIdx >= 0)
			{
				std::cerr << strus::string_format( "execute test %d only", testIdx) << std::endl;
			}
		}
		// Generate the sets:
		if (verbose)
		{
			std::cerr << "generating input ..." << std::endl;
		}
		int ii=2;
		std::vector<std::vector<int> > sets;
		for (; ii<nofSets; ++ii)
		{
			if (ii <= maxNumber)
			{
				sets.push_back( generateSet( ii, maxNumber+1));
			}
			else
			{
				std::vector<int> cc = joinSets( sets[ g_random.get( 0,sets.size())], sets[ g_random.get( 0,sets.size())]);
				while (g_random.get( 0,3) == 1)
				{
					cc = joinSets( cc, sets[ g_random.get( 0,sets.size())]);
				}
				sets.push_back( removePrimes( cc));
			}
		}
		shuffle( sets);

		strus::MinimalCoverData testdata( sets, &g_errorbuf);
		// Corner cases:
		if (verbose)
		{
			std::cerr << "test some corner cases ..." << std::endl;
		}{
			std::vector<int> input;
			if (testIdx != -1 && testdata.minimalCoverApproximation( input).size() != 0)
			{
				throw std::runtime_error("test with empty input failed");
			}
		}{
			std::vector<int> input;
			input.push_back( g_random.get( 2, maxNumber+1));

			if (testIdx != -1 && testdata.minimalCoverApproximation( input).size() != 1)
			{
				throw std::runtime_error("test with one element input failed");
			}
		}{
			std::vector<int> input;
			input.push_back( 3);
			input.push_back( 6);
			input.push_back( 9);

			if (testIdx != -1 && testdata.minimalCoverApproximation( input).size() != 1)
			{
				throw std::runtime_error("test with 3 multiplicatives of 3 as input failed");
			}
		}{
			std::vector<int> input;
			input.push_back( 3);
			input.push_back( 5);

			if (testIdx != -1 && testdata.minimalCoverApproximation( input).size() != 2)
			{
				throw std::runtime_error("test with 2 disjoint sets as input failed");
			}
		}
		// Run the tests:
		if (verbose)
		{
			std::cerr << "run the random generated tests ..." << std::endl;
		}
		int ti = 0, te = nofTests;
		int nofErrors = 0;
		int nofUntolerableErrors = 0;
		std::vector<std::vector<int> > inputs;
		std::vector<std::vector<int> > primecovers;
		std::vector<int> results;

		for (ti=0; ti != te; ++ti)
		{
			std::vector<int> input;
			int ei=1,ee = 1+g_random.get( 0, 1+g_random.get( 0, nofSets));
			for (; ei != ee; ++ei)
			{
				int num = 2+g_random.get( 0, maxNumber-1);
				input.push_back( num);
			}
			inputs.push_back( input);
		}
		std::clock_t start = std::clock();
		for (ti=0; ti != te; ++ti)
		{
			results.push_back( testdata.minimalCoverApproximation( inputs[ti]).size());
		}
		double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
		std::cerr << strus::string_format( "evaluated %d random cover calculations with %d sets of numbers [2..%d] in %.3f seconds", nofTests, nofSets, maxNumber, (float)duration) << std::endl;

		for (ti=0; ti != te; ++ti)
		{
			primecovers.push_back( calculateSmallestPrimeNumberCoverAppriximation( inputs[ti]));
		}
		for (ti=0; ti != te; ++ti)
		{
			const std::vector<int>& input = inputs[ ti];
			if (testIdx == -1 || testIdx == ti)
			{
				if (verbose)
				{
					std::cerr << "run test " << ti << ": {";
					std::vector<int>::const_iterator ai = input.begin(), ae = input.end();
					for (; ai != ae; ++ai)
					{
						std::cerr << " " << *ai;
					}
					std::cerr << "}";
				}
				int result = results[ ti];
				const std::vector<int>& primecover = primecovers[ ti];
				int expected = primecover.size();
				if (result <= 0)
				{
					if (g_errorbuf.hasError())
					{
						throw std::runtime_error( g_errorbuf.fetchError());
					}
				}
				if (result < expected)
				{
					if (verbose)
					{
						std::cerr << "test apprimation missed a solution" << std::endl;
					}
				}
				else if (result > expected)
				{
					nofErrors += 1;
					int toleratedResult = expected * (1.0 + errorTolerance) + 0.5;
					if (toleratedResult == expected)
					{
						toleratedResult += 1;//... one miss is always allowed
					}
					if (result > toleratedResult)
					{
						nofUntolerableErrors += 1;
						std::cerr << "untolerable error in approximation result " << result << ", expected " << expected << std::endl;
					}
				}
				if (verbose)
				{
					if (result != expected)
					{
						std::cerr << " => " << result
							  << " | " << expected
							  << " {" << integerListToString( primecover, ", ") << "}"
							  << std::endl;
					}
					else
					{
						std::cerr << " => " << result << std::endl;
					}
				}
			}
		}
		if (nofUntolerableErrors)
		{
			std::cerr << "FAILED " << nofUntolerableErrors << " beyond 10% tolerance, " << nofErrors << " acceptable" << std::endl;
			return 3;
		}
		else
		{
			if (nofErrors)
			{
				std::cerr << strus::string_format("number of tests with a slight but tolerable deviation (within %d%%) from the expected value: %d", (int)(100*errorTolerance), nofErrors) << std::endl;
			}
			if (g_errorbuf.hasError())
			{
				throw std::runtime_error( g_errorbuf.fetchError());
			}
			std::cerr << "OK" << std::endl;
			return 0;
		}
	}
	catch (const std::bad_alloc& err)
	{
		std::cerr << "ERROR " << err.what() << std::endl;
		return 2;
	}
	catch (const std::exception& err)
	{
		std::cerr << "ERROR " << err.what() << std::endl;
		return 1;
	}
}


