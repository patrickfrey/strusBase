/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/error.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/base/thread.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/base/fileio.hpp"
#include "strus/base/string_format.hpp"
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <cstdlib>

static strus::ErrorBufferInterface* g_errhnd = 0;

void runThread( int threadid)
{
	enum {NofRuns=1000};
	for (int ii=0; ii<NofRuns; ++ii)
	{
		g_errhnd->info( "T %d %d", threadid, ii);
	}
	std::vector<std::string> infos = g_errhnd->fetchInfo();
	if ((int)infos.size() != NofRuns)
	{
		throw std::runtime_error( strus::string_format( "number of info messages thread %d not as expected, read %d expected %d", threadid, (int)infos.size(), (int)NofRuns));
	}
	std::vector<std::string>::const_iterator
		mi = infos.begin(), me = infos.end();
	for (int ii=0; mi != me; ++mi,++ii)
	{
		int threadid_;
		int ii_;
		if (2!=std::sscanf( mi->c_str(), "T %d %d", &threadid_, &ii_))
		{
			if (threadid != threadid_ || ii != ii_)
			{
				throw std::runtime_error( strus::string_format( "info message %d of thread %d not as expected, read index %d and thread id %d", ii, threadid, ii_, threadid_));
			}
		}
	}
}

void printUsage()
{
	std::cout << "Usage: testErrorBuffer [-h] [ <outputdir> ]" << std::endl;
	std::cout << "<outputdir>  :Directory Where to write logs to" << std::endl;
}

int main( int argc, const char** argv)
{
	FILE* logfile = 0;
	try
	{
		std::string logfilenamebuf;
		const char* logfilename = 0;
		if (argc > 1)
		{
			if (0==std::strcmp( "-h", argv[1]))
			{
				printUsage();
				return 0;
			}
			logfilenamebuf = strus::joinFilePath( argv[1], "LOG");
			logfilename = logfilenamebuf.c_str();
			logfile = std::fopen( logfilename, "w");
			if (argc > 2)
			{
				printUsage();
				throw std::runtime_error( "too many arguments");
			}
		}
		if (!logfile)
		{
			std::cerr << "printing logs to stderr ..." << std::endl;
		}
		int maxNofThreads = 10;
		strus::local_ptr<strus::DebugTraceInterface> debugtrace( strus::createDebugTrace_standard( maxNofThreads));
		g_errhnd = strus::createErrorBuffer_standard( logfile?logfile:stderr, maxNofThreads, debugtrace.release());
		if (!g_errhnd) throw std::runtime_error( "failed to create error buffer");

		strus::thread_group tgroup;
		int ti=0, te=maxNofThreads-1;
		for (; ti != te; ++ti)
		{
			tgroup.add_thread( new strus::thread( runThread, ti+1));
		}
		tgroup.join_all();
		if (g_errhnd->hasError())
		{
			throw std::runtime_error( "test failed");
		}
		std::cerr << "OK" << std::endl;
		if (g_errhnd) delete g_errhnd;
		if (logfile) std::fclose( logfile);
		return 0;
	}
	catch (const std::bad_alloc& err)
	{
		std::cerr << "ERROR " << err.what() << std::endl;
	}
	catch (const std::exception& err)
	{
		if (g_errhnd && g_errhnd->hasError())
		{
			std::cerr << "ERROR " << err.what() << ", " << g_errhnd->fetchError() << std::endl;
		}
		else
		{
			std::cerr << "ERROR " << err.what() << std::endl;
		}
	}
	if (g_errhnd) delete g_errhnd;
	if (logfile) std::fclose( logfile);
	return -1;
}


