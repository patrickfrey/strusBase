/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structured error codes for strus components
/// \file errorCodes.hpp
#ifndef _STRUS_ERROR_CODES_HPP_INCLUDED
#define _STRUS_ERROR_CODES_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{
	/// \brief Application error codes beyond errno (>= 200)
	enum ErrorCode
	{
		ErrorCodeUnknown=0,
		ErrorCodeErrno=201,
		ErrorCodeOutOfMem=202,
		ErrorCodeDataCorruption=203,
		ErrorCodeMutexLockFailed=214,
		ErrorCodeLogicError=215,
		ErrorCodeUncaughtException=216,
		ErrorCodeAvailability=220,
		ErrorCodeNotAllowed=221,
		ErrorCodeProtocolError=222,
		ErrorCodePlatformIncompatibility=223,
		ErrorCodePlatformRequirements=224,
		ErrorCodeSyntax=228,
		ErrorCodeNotFound=229,
		ErrorCodeIOError=230,
		ErrorCodeVersionMismatch=231,
		ErrorCodeInvalidArgument=232,
		ErrorCodeInvalidRegex=233,
		ErrorCodeInvalidOperation=234,
		ErrorCodeInvalidFilePath=235,
		ErrorCodeInvalidRequest=236,
		ErrorCodeLoadModuleFailed=237,
		ErrorCodeNotImplemented=238,
		ErrorCodeIncompleteInitialization=239,
		ErrorCodeIncompleteDefinition=240,
		ErrorCodeIncompleteConfiguration=241,
		ErrorCodeDuplicateDefinition=242,
		ErrorCodeBindingLanguageError=251,
		ErrorCodeUnknownIdentifier=252,
		ErrorCodeOperationOrder=253,
		ErrorCodeValueOutOfRange=254,
		ErrorCodeMaxLimitReached=255,
		ErrorCodeBufferOverflow=256,
		ErrorCodeMaxNofItemsExceeded=257,
		ErrorCodeMaxRecursionDepht=258,
		ErrorCodeRefusedDueToComplexity=259,
		ErrorCodeRuntimeError=260,
		ErrorCodeIncompleteRequest=265,
		ErrorCodeIncompleteResult=266,
		ErrorCodeUnexpectedEof=271,
		ErrorCodeHiddenError=272,
		ErrorCodeInputFormat=281,
		ErrorCodeEncoding=282,
		ErrorCodeRequestResolveError=283,
		ErrorCodeNotAcceptable=284,
		ErrorCodeDelegateRequestFailed=285,
		ErrorCodeServiceShutdown=286,
		ErrorCodeServiceTemporarilyUnavailable=287
	};
}//namespace
#endif



