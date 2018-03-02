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
	enum ErrorComponent
	{
		StrusComponentUnknown=0,
		StrusComponentBase=1,
		StrusComponentCore=2,
		StrusComponentAnalyzer=3,
		StrusComponentTrace=11,
		StrusComponentModule=12,
		StrusComponentRpc=13,
		StrusComponentVector=21,
		StrusComponentPattern=22,
		StrusComponentUtilities=31,
		StrusComponentBindings=32,
		StrusComponentWebService=33,
		StrusComponentOther=41
	};

	enum ErrorOperation
	{
		ErrorOperationUnknown=0,
		ErrorOperationReadFile=1,
		ErrorOperationWriteFile=2,
		ErrorOperationConnect=3,
		ErrorOperationReadEnv=4,
		ErrorOperationLog=5,
		ErrorOperationCreateStorage=11,
		ErrorOperationDestroyStorage=12,
		ErrorOperationOpenStorage=13,
		ErrorOperationCloseStorage=14,
		ErrorOperationSwapStorage=15,
		ErrorOperationReadStorage=16,
		ErrorOperationWriteStorage=17,
		ErrorOperationCommitStorage=18,
		ErrorOperationRollbackStorage=19,
		ErrorOperationCheckStorage=20,
		ErrorOperationLoadModule=25,
		ErrorOperationParse=31,
		ErrorOperationBuildData=32,
		ErrorOperationBuildResult=33,
		ErrorOperationConfiguration=34,
		ErrorOperationCallIndirection=35,
		ErrorOperationConversion=36,
		ErrorOperationScanInput=37
	};
	enum {MinErrorOperationWithErrnoCode=1,MaxErrorOperationWithErrnoCode=10};

	/// \brief Application sub error codes beyond errno
	enum ErrorCause
	{
		ErrorCauseUnknown=0,
		ErrorCauseErrno=201,
		ErrorCauseOutOfMem=202,
		ErrorCauseDataCorruption=203,
		ErrorCauseMutexLockFailed=204,
		ErrorCauseLogicError=205,
		ErrorCauseUncaughtException=206,
		ErrorCauseAvailability=220,
		ErrorCauseNotAllowed=221,
		ErrorCauseProtocolError=222,
		ErrorCausePlatformIncompatibility=223,
		ErrorCausePlatformRequirements=224,
		ErrorCauseSyntax=228,
		ErrorCauseNotFound=229,
		ErrorCauseIOError=230,
		ErrorCauseVersionMismatch=231,
		ErrorCauseInvalidArgument=232,
		ErrorCauseInvalidRegex=233,
		ErrorCauseInvalidOperation=234,
		ErrorCauseNotImplemented=235,
		ErrorCauseIncompleteInitialization=236,
		ErrorCauseIncompleteDefinition=237,
		ErrorCauseBindingLanguageError=251,
		ErrorCauseUnknownIdentifier=252,
		ErrorCauseOperationOrder=253,
		ErrorCauseValueOutOfRange=254,
		ErrorCauseMaxLimitReached=255,
		ErrorCauseBufferOverflow=256,
		ErrorCauseMaxNofItemsExceeded=257,
		ErrorCauseMaxRecursionDepht=258,
		ErrorCauseRuntimeError=259,
		ErrorCauseIncompleteRequest=265,
		ErrorCauseIncompleteResult=266,
		ErrorCauseUnexpectedEof=271,
		ErrorCauseHiddenError=272,
		ErrorCauseInputFormat=281,
		ErrorCauseEncoding=282,
		ErrorCauseRequestResolveError=283
	};

	struct ErrorCode
	{
		int value;

		ErrorCode( const ErrorCode& o)
			:value(o.value){}
		ErrorCode( ErrorComponent comp_, ErrorOperation op_, ErrorCause cause_)
			:value( (int)comp_ * 1000000 + (int)op_ * 1000 + (int)cause_){}
		ErrorCode( ErrorComponent comp_, ErrorOperation op_, int errno_)
			:value( (int)comp_ * 1000000 + (int)op_ * 1000 + (int)errno_){}
		ErrorCode( int value_)
			:value(value_){}
		operator int() const				{return value;}
		int operator*() const				{return value;}

		ErrorComponent component() const		{return (ErrorComponent)((value / 1000000) % 1000);}
		ErrorOperation operation() const		{return (ErrorOperation)((value /    1000) % 1000);}
		ErrorCause cause() const			{int ec = value % 1000; return ec ? (ec < 200 ? ErrorCauseErrno : (ErrorCause)(ec)) : ErrorCauseUnknown;}
		int syserrno() const				{int ec = value % 1000; return ec < 200 ? ec : 0;}

		void setComponent( const ErrorComponent& comp_)	{if (comp_) value = value -  ((value / 1000000) % 1000) + (int)comp_ * 1000000;}
		void setOperation( const ErrorOperation& op_)	{if (op_) value = value -    ((value /    1000) % 1000) + (int)op_   *    1000;}
		void setCause( const ErrorCause& cause_)	{if (cause_) value = value - ((value          ) % 1000) + (int)cause_;}
		void setSysErrno( int errno_)			{if (errno_) value = value - ((value          ) % 1000) + (int)errno_;}
	};
}//namespace
#endif



