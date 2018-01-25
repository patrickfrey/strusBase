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
		ErrorOperationCallIndirection=33,
		ErrorOperationConversion=34,
		ErrorOperationScanInput=35
	};
	enum {MinErrorOperationWithErrnoCode=1,MaxErrorOperationWithErrnoCode=10};

	/// \brief Application sub error codes used for ErrorOperation not inbetween MinErrorOperationWithErrnoCode and MaxErrorOperationWithErrnoCode (these use errno codes as cause)
	enum ErrorCause
	{
		ErrorCauseUnknown=0,
		ErrorCauseErrno=1,
		ErrorCauseOutOfMem=2,
		ErrorCauseDataCorruption=3,
		ErrorCauseAvailability=4,
		ErrorCauseNotAllowed=5,
		ErrorCauseProtocolError=6,
		ErrorCauseSyntax=7,
		ErrorCauseNotFound=8,
		ErrorCauseIOError=9,
		ErrorCauseVersionMismatch=10,
		ErrorCauseInvalidArgument=11,
		ErrorCauseInvalidRegex=12,
		ErrorCauseInvalidOperation=13,
		ErrorCauseNotImplemented=14,
		ErrorCauseIncompleteDefinition=15,
		ErrorCauseBindingLanguageError=16,
		ErrorCauseUnknownIdentifier=17,
		ErrorCauseOperationOrder=18,
		ErrorCauseValueOutOfRange=19,
		ErrorCauseMaximumLimitReached=20,
		ErrorCauseBufferOverflow=21,
		ErrorCauseMaxNofItemsExceeded=22,
		ErrorCauseLogicError=24,
		ErrorCauseRuntimeError=25,
		ErrorCauseUncaughtException=26,
		ErrorCauseIncompleteRequest=27,
		ErrorCauseUnexpectedEof=28,
		ErrorCausePlatformIncompatibility=29,
		ErrorCausePlatformRequirements=30,
		ErrorCauseHiddenError=31,
		ErrorCauseInputFormat=41,
		ErrorCauseEncoding=42
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
		bool hasSysErrno() const			{int op = (int)operation(); return (op >= MinErrorOperationWithErrnoCode && op <= MaxErrorOperationWithErrnoCode);}
		ErrorCause cause() const			{return hasSysErrno() ? ErrorCauseErrno : (ErrorCause)(value % 1000);}
		int syserrno() const				{return hasSysErrno() ? (value % 1000) : 0;}

		void setComponent( const ErrorComponent& comp_)	{if (comp_) value = value -  ((value / 1000000) % 1000) + (int)comp_ * 1000000;}
		void setOperation( const ErrorOperation& op_)	{if (op_) value = value -    ((value /    1000) % 1000) + (int)op_   *    1000;}
		void setCause( const ErrorCause& cause_)	{if (cause_) value = value - ((value          ) % 1000) + (int)cause_;}
		void setSysErrno( int errno_)			{if (errno_) value = value - ((value          ) % 1000) + (int)errno_;}
	};
}//namespace
#endif



