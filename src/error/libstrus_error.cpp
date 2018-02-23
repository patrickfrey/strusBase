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
#include <stdexcept>
#include <cstdio>
#include <cstring>

using namespace strus;

DLL_PUBLIC ErrorBufferInterface* strus::createErrorBuffer_standard( FILE* logfilehandle, std::size_t maxNofThreads_)
{
	try
	{
		return new ErrorBuffer( logfilehandle, maxNofThreads_);
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

DLL_PUBLIC const char* strus::errorComponentName( ErrorComponent component)
{
	switch (component)
	{
		case StrusComponentUnknown: return "unknown";
		case StrusComponentBase: return "base";
		case StrusComponentCore: return "core";
		case StrusComponentAnalyzer: return "analyzer";
		case StrusComponentTrace: return "trace";
		case StrusComponentModule: return "module";
		case StrusComponentRpc: return "rpc";
		case StrusComponentVector: return "vector";
		case StrusComponentPattern: return "pattern";
		case StrusComponentUtilities: return "utilities";
		case StrusComponentBindings: return "bindings";
		case StrusComponentWebService: return "webservice";
		default: return "other";
	}
}

DLL_PUBLIC const char* strus::errorCauseMessage( ErrorCause cause)
{
	if ((int)cause < 200) return ::strerror((int)cause);
	switch (cause)
	{
		case ErrorCauseUnknown:			return _TXT("unknown");
		case ErrorCauseErrno:			return _TXT("unknown");
		case ErrorCauseOutOfMem:		return _TXT("out of memory");
		case ErrorCauseDataCorruption:		return _TXT("data corruption");
		case ErrorCauseMutexLockFailed:		return _TXT("mutex lock failed");
		case ErrorCauseLogicError:		return _TXT("logic error");
		case ErrorCauseUncaughtException:	return _TXT("uncaught exception");
		case ErrorCauseAvailability:		return _TXT("service not available");
		case ErrorCauseNotAllowed:		return _TXT("access not allowed");
		case ErrorCauseProtocolError:		return _TXT("protocol error");
		case ErrorCausePlatformIncompatibility:	return _TXT("platform incompatibility");
		case ErrorCausePlatformRequirements:	return _TXT("unsatisfied platform requirements");
		case ErrorCauseSyntax:			return _TXT("syntax error");
		case ErrorCauseNotFound:		return _TXT("resource not found");
		case ErrorCauseIOError:			return _TXT("IO error");
		case ErrorCauseVersionMismatch:		return _TXT("version mismatch");
		case ErrorCauseInvalidArgument:		return _TXT("invalid argument");
		case ErrorCauseInvalidRegex:		return _TXT("invalid regular expression");
		case ErrorCauseInvalidOperation:	return _TXT("invalid operation");
		case ErrorCauseNotImplemented:		return _TXT("not implemented");
		case ErrorCauseIncompleteDefinition:	return _TXT("incomplete definition");
		case ErrorCauseBindingLanguageError:	return _TXT("binding language error");
		case ErrorCauseUnknownIdentifier:	return _TXT("unknown identifier");
		case ErrorCauseOperationOrder:		return _TXT("invalid order of operations");
		case ErrorCauseValueOutOfRange:		return _TXT("value out of range");
		case ErrorCauseMaximumLimitReached:	return _TXT("maximum limit reached");
		case ErrorCauseBufferOverflow:		return _TXT("buffer overflow");
		case ErrorCauseMaxNofItemsExceeded:	return _TXT("maximum number if items exceeded");
		case ErrorCauseRuntimeError:		return _TXT("runtime error");
		case ErrorCauseIncompleteRequest:	return _TXT("incomplete request");
		case ErrorCauseUnexpectedEof:		return _TXT("unexpected end of file/input");
		case ErrorCauseHiddenError:		return _TXT("hidden error (interface does not allow to see details)");
		case ErrorCauseInputFormat:		return _TXT("input format");
		case ErrorCauseEncoding:		return _TXT("character encoding error");
		case ErrorCauseRequestResolveError:	return _TXT("cannot resolve request (HTTP 404)");
		default: return _TXT("unknown");
	}
}


