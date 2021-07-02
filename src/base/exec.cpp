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

static int execve_tostring_( const char* filename, const char* const argv[], const std::map<std::string,std::string>* env, std::string& output)
{
	int rt = 0;
	int pipefd[2];
	int status = pipe( pipefd);
	if (status < 0)
	{
		return errno;
	}
	pid_t pid = ::fork();
	if (pid < 0)
	{
		return errno;
	}
	else if (pid == 0)
	{
		// ... child
		close(pipefd[0]);    // close stdout of child
		dup2(pipefd[1], 1);  // send stdout to the pipe
		close(pipefd[1]);    // close read

		if (env)
		{
			try
			{
				// Build environment string:
				std::vector<std::string> envstrar;
				std::vector<const char*> envar;
				std::map<std::string,std::string>::const_iterator mi = env->begin(), me = env->end();
				for (; mi != me; ++mi)
				{
					std::string envstr;
					envstr.append( mi->first);
					envstr.push_back( '=');
					envstr.append( mi->second);
					envstrar.push_back( envstr);
				}
				std::vector<std::string>::const_iterator ei = envstrar.begin(), ee = envstrar.end();
				for (; ei != ee; ++ei)
				{
					envar.push_back( ei->c_str());
				}
				envar.push_back( (const char*)0);

				// Execute command with environment:
				::execve( filename, const_cast<char* const*>(argv), const_cast<char* const*>( envar.data()));
			}
			catch (const std::bad_alloc&)
			{
				exit( ENOMEM);
			}
		}
		else
		{
			// Execute command:
			::execv( filename, const_cast<char* const*>(argv));
		}
		rt = errno;
		exit( rt);
	}
	else
	{
		// ... parent
		char buffer[1024];
		close(pipefd[1]);  // close the write end of the pipe in the parent
		int size;

		try
		{
			while (0!=(size=::read(pipefd[0], buffer, sizeof(buffer))))
			{
				output.append( buffer, size);
			}
			rt = errno;
			close(pipefd[0]);
		}
		catch (const std::bad_alloc&)
		{
			close(pipefd[0]);
			return ENOMEM;
		}
		pid_t wp;
		while (pid != (wp=::waitpid( pid, &status, WUNTRACED)))
		{
			sleep( 1);
		}
		if (WIFEXITED(status) && !rt)
		{
			rt = WEXITSTATUS(status);
		}
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



