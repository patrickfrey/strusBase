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
		ErrorOperationConfiguration=33,
		ErrorOperationCallIndirection=34,
		ErrorOperationConversion=35,
		ErrorOperationScanInput=36
	};
	enum {MinErrorOperationWithErrnoCode=1,MaxErrorOperationWithErrnoCode=10};

	/// \brief Application sub error codes beyond errno
	enum ErrorCause
	{
		ErrorCauseUnknown=200,
		ErrorCauseErrno=201,
		ErrorCauseOutOfMem=202,
		ErrorCauseDataCorruption=203,
		ErrorCauseAvailability=204,
		ErrorCauseNotAllowed=205,
		ErrorCauseProtocolError=206,
		ErrorCauseSyntax=207,
		ErrorCauseNotFound=208,
		ErrorCauseIOError=209,
		ErrorCauseVersionMismatch=210,
		ErrorCauseInvalidArgument=211,
		ErrorCauseInvalidRegex=212,
		ErrorCauseInvalidOperation=213,
		ErrorCauseNotImplemented=214,
		ErrorCauseIncompleteDefinition=215,
		ErrorCauseBindingLanguageError=216,
		ErrorCauseUnknownIdentifier=217,
		ErrorCauseOperationOrder=218,
		ErrorCauseValueOutOfRange=219,
		ErrorCauseMaximumLimitReached=220,
		ErrorCauseBufferOverflow=221,
		ErrorCauseMaxNofItemsExceeded=222,
		ErrorCauseLogicError=224,
		ErrorCauseRuntimeError=225,
		ErrorCauseUncaughtException=226,
		ErrorCauseIncompleteRequest=227,
		ErrorCauseUnexpectedEof=228,
		ErrorCausePlatformIncompatibility=229,
		ErrorCausePlatformRequirements=230,
		ErrorCauseHiddenError=231,
		ErrorCauseInputFormat=241,
		ErrorCauseEncoding=242
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
		ErrorCause cause() const			{int ec = value % 1000; return ec < 200 ? ErrorCauseErrno : (ErrorCause)(ec);}
		int syserrno() const				{int ec = value % 1000; return ec < 200 ? ec : 0;}

		void setComponent( const ErrorComponent& comp_)	{if (comp_) value = value -  ((value / 1000000) % 1000) + (int)comp_ * 1000000;}
		void setOperation( const ErrorOperation& op_)	{if (op_) value = value -    ((value /    1000) % 1000) + (int)op_   *    1000;}
		void setCause( const ErrorCause& cause_)	{if (cause_) value = value - ((value          ) % 1000) + (int)cause_;}
		void setSysErrno( int errno_)			{if (errno_) value = value - ((value          ) % 1000) + (int)errno_;}
	};
}//namespace
#endif



