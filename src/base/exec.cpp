/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Execution of a command appending results to string
/// \remark Currently we are only providing a POSIX solution. boost::process could be a substitute, but is only available for boost >= 1.64
#include "strus/base/exec.hpp"
#include "strus/base/dll_tags.hpp"
#include <cerrno>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
using namespace strus;

#if 0 && BOOST_VERSION >= 106400
// ... Boost implementation trial not tested, therefore does not work
#include <boost/process.hpp>

DLL_PUBLIC int strus::execv_tostring( const char* filename, const char* argv[], std::string& output)
{
	try
	{
		std::ostringstream out;
		boost::process::ipstream pipe_stream;
		std::string cmdstr = filename;
		int ai = 0;
		while (argv[ai])
		{
			cmdstr.push_back( ' ');
			cmdstr.push_back( '"');
			cmdstr.append( argv[ai]);
			cmdstr.push_back( '"');
		}
		boost::process::child chld( cmdstr.c_str(), std_out > pipe_stream);

		std::string line;
		while (pipe_stream && std::getline( pipe_stream, line) && !line.empty())
		{
			out << line << std::endl;
		}
		chld.wait();
		output.append( out.str());
	}
	catch (const std::bad_alloc&)
	{
		::close( pipefd[0]);
		return ENOMEM;
	}
}
DLL_PUBLIC int strus::execve_tostring( const char* filename, const char* const argv[], const std::map<std::string,std::string>& env, std::string& output)
{
	!!! NOT IMPLEMENTED YET !!!
}
#else

#define READ_END	0
#define WRITE_END	1
#define STDOUT_FILENO	1

static int execve_tostring_( const char* filename, const char* const argv[], const std::map<std::string,std::string>* env, std::string& output)
{
	int rt = 0;
	int pipefd[ 2];
	int status = ::pipe( pipefd);
	if (status < 0)
	{
		return errno;
	}
	pid_t pid = ::fork();
	if (pid == 0)
	{
		// ... child
		::dup2( pipefd[ WRITE_END], STDOUT_FILENO);
		::close( pipefd[ READ_END]);
		::close( pipefd[ WRITE_END]);
		if (env)
		{
			std::map<std::string,std::string>::const_iterator ei = env->begin(), ee = env->end();
			for (; ei != ee; ++ei)
			{
				::setenv( ei->first.c_str(), ei->second.c_str(), 1/*overwrite*/);
			}
		}
		int res = ::execv( filename, const_cast<char* const*>(argv));
		if (res == -1)
		{
			rt = errno;
		}
		::exit( rt);
	}
	else if (pid > 0)
	{
		// ... parent
		char buffer[ 1024];
		::close( pipefd[ WRITE_END]);
		ssize_t size;
		while (0 < (size=::read( pipefd[ READ_END], buffer, sizeof(buffer))))
		{
			try
			{
				output.append( buffer, size);
			}
			catch (const std::bad_alloc&)
			{
				rt = ENOMEM;
				break;
			}
		}
		if (!rt && size == -1)
		{
			rt = errno;
		}
		::close( pipefd[ READ_END]);

		pid_t wp;
		while (pid != (wp=::waitpid( pid, &status, WUNTRACED)))
		{
			::sleep( 1);
		}
		if (WIFEXITED(status) && !rt)
		{
			rt = WEXITSTATUS(status);
		}
	}
	else
	{
		rt = errno;
	}
	return rt;
}

DLL_PUBLIC int strus::execv_tostring( const char* filename, const char* const argv[], std::string& output)
{
	return execve_tostring_( filename, argv, NULL, output);
}

DLL_PUBLIC int strus::execve_tostring( const char* filename, const char* const argv[], const std::map<std::string,std::string>& env, std::string& output)
{
	return execve_tostring_( filename, argv, &env, output);
}

#endif



