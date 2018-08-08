/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus standard error buffer library
/// \file libstrus_error.cpp
#include "strus/lib/error.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include "errorBuffer.hpp"
#include "debugTrace.hpp"
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>

using namespace strus;

DLL_PUBLIC DebugTraceInterface* strus::createDebugTrace_standard( std::size_t maxNofThreads_)
{
	try
	{
		return new DebugTrace( maxNofThreads_);
	}
	catch (const std::bad_alloc&)
	{
		return 0;
	}
	catch (const std::exception& err)
	{
		return 0;
	}
}

static std::string encodeMessage( const std::string& msg)
{
	std::string rt;
	std::string::const_iterator mi = msg.begin(), me = msg.end();
	for (; mi != me; ++mi)
	{
		if ((unsigned char)*mi < 32) rt.push_back( ' '); else rt.push_back( *mi);
	}
	return rt;
}

DLL_PUBLIC bool strus::dumpDebugTrace( DebugTraceInterface* debugTrace, const char* filename)
{
	try
	{
		std::vector<DebugTraceMessage> msglist = debugTrace->fetchMessages();
		std::vector<DebugTraceMessage>::const_iterator mi = msglist.begin(), me = msglist.end();
		if (filename)
		{
			std::ofstream ostrm( filename, std::ios::binary);
			for (; mi != me; ++mi)
			{
				ostrm << mi->typeName() << ' ' << mi->component() << ' ' << mi->id() << ' ' << encodeMessage(mi->content()) << '\n';
			}
		}
		else
		{
			for (; mi != me; ++mi)
			{
				std::cerr << mi->typeName() << ' ' << mi->component() << ' ' << mi->id() << ' ' << encodeMessage(mi->content()) << '\n';
			}
		}
		return true;
	}
	catch (const std::bad_alloc&)
	{
		return false;
	}
	catch (const std::exception& err)
	{
		return false;
	}
}

DLL_PUBLIC ErrorBufferInterface* strus::createErrorBuffer_standard( FILE* logfilehandle, std::size_t maxNofThreads_, DebugTraceInterface* dbgtrace_)
{
	try
	{
		return new ErrorBuffer( logfilehandle, maxNofThreads_, dbgtrace_);
	}
	catch (const std::bad_alloc&)
	{
		fprintf( logfilehandle?logfilehandle:stderr, "%s", _TXT("out of memory creating error buffer\n"));
		return 0;
	}
	catch (const std::exception& err)
	{
		fprintf( logfilehandle?logfilehandle:stderr, _TXT("exception creating error buffer: %s\n"), err.what());
		return 0;
	}
}

DLL_PUBLIC int strus::errorCodeFromMessage( char const*& msgitr)
{
	return ErrorBuffer::nextErrorCode( msgitr);
}

DLL_PUBLIC void strus::removeErrorCodesFromMessage( char* msg)
{
	ErrorBuffer::removeErrorCodes( msg);
}

DLL_PUBLIC const char* strus::errorCodeToString( int errcode)
{
	if (errcode < 200) return ::strerror( errcode);
	switch ((ErrorCode)errcode)
	{
		case ErrorCodeUnknown:			return _TXT("unknown");
		case ErrorCodeErrno:			return _TXT("unknown");
		case ErrorCodeOutOfMem:			return _TXT("out of memory");
		case ErrorCodeDataCorruption:		return _TXT("data corruption");
		case ErrorCodeMutexLockFailed:		return _TXT("mutex lock failed");
		case ErrorCodeLogicError:		return _TXT("logic error");
		case ErrorCodeUncaughtException:	return _TXT("uncaught exception");
		case ErrorCodeAvailability:		return _TXT("service not available");
		case ErrorCodeNotAllowed:		return _TXT("access not allowed");
		case ErrorCodeProtocolError:		return _TXT("protocol error");
		case ErrorCodePlatformIncompatibility:	return _TXT("platform incompatibility");
		case ErrorCodePlatformRequirements:	return _TXT("unsatisfied platform requirements");
		case ErrorCodeSyntax:			return _TXT("syntax error");
		case ErrorCodeNotFound:			return _TXT("resource not found");
		case ErrorCodeIOError:			return _TXT("IO error");
		case ErrorCodeVersionMismatch:		return _TXT("version mismatch");
		case ErrorCodeInvalidArgument:		return _TXT("invalid argument");
		case ErrorCodeInvalidRegex:		return _TXT("invalid regular expression");
		case ErrorCodeInvalidOperation:		return _TXT("invalid operation");
		case ErrorCodeInvalidFilePath:		return _TXT("invalid or not allowed file path");
		case ErrorCodeLoadModuleFailed:		return _TXT("failed to load module");
		case ErrorCodeNotImplemented:		return _TXT("not implemented");
		case ErrorCodeIncompleteInitialization:	return _TXT("incomplete initialization of object called");
		case ErrorCodeIncompleteDefinition:	return _TXT("incomplete definition");
		case ErrorCodeIncompleteConfiguration:	return _TXT("incomplete configuration");
		case ErrorCodeBindingLanguageError:	return _TXT("binding language error");
		case ErrorCodeUnknownIdentifier:	return _TXT("unknown identifier");
		case ErrorCodeOperationOrder:		return _TXT("invalid order of operations");
		case ErrorCodeValueOutOfRange:		return _TXT("value out of range");
		case ErrorCodeMaxLimitReached:		return _TXT("maximum limit reached");
		case ErrorCodeBufferOverflow:		return _TXT("buffer overflow");
		case ErrorCodeMaxNofItemsExceeded:	return _TXT("maximum number if items exceeded");
		case ErrorCodeMaxRecursionDepht:	return _TXT("maximum depth of recursion reached");
		case ErrorCodeRuntimeError:		return _TXT("runtime error");
		case ErrorCodeIncompleteRequest:	return _TXT("incomplete request");
		case ErrorCodeIncompleteResult:		return _TXT("incomplete result");
		case ErrorCodeUnexpectedEof:		return _TXT("unexpected end of file/input");
		case ErrorCodeHiddenError:		return _TXT("hidden error (interface does not allow to see details)");
		case ErrorCodeInputFormat:		return _TXT("input format");
		case ErrorCodeEncoding:			return _TXT("character encoding error");
		case ErrorCodeRequestResolveError:	return _TXT("cannot resolve request");
		case ErrorCodeNotAcceptable:		return _TXT("possible response not acceptable by client");
		default: return _TXT("unknown");
	}
}


