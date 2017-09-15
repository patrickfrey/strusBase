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
#include <string>
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
		std::string cmdstr = cmd.tostring();
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
		close(pipefd[0]);
		return 12/*ENOMEM*/;
	}
}
#else
DLL_PUBLIC int strus::execv_tostring( const char* filename, const char* const argv[], std::string& output)
{
	int rt = 0;
	int pipefd[2];
	int status = pipe( pipefd);
	if (status < 0)
	{
		return errno;
	}
	pid_t pid = fork();
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

		::execv( filename, const_cast<char* const*>(argv));
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
			while (0!=(size=read(pipefd[0], buffer, sizeof(buffer))))
			{
				output.append( buffer, size);
			}
			close(pipefd[0]);
		}
		catch (const std::bad_alloc&)
		{
			close(pipefd[0]);
			return 12/*ENOMEM*/;
		}
		pid_t wp;
		while (pid != (wp=::waitpid( pid, &status, WUNTRACED)))
		{
			sleep( 1);
		}
		if (WIFEXITED(status))
		{
			rt = WEXITSTATUS(status);
		}
	}
	return rt;
}
#endif



