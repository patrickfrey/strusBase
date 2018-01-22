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
		StrusComponentTrace=4,
		StrusComponentModule=5,
		StrusComponentRpc=6,
		StrusComponentVector=7,
		StrusComponentPattern=8,
		StrusComponentUtilities=9,
		StrusComponentBindings=10,
		StrusComponentWebService=11,
		StrusComponentOther=50
	};

	enum ErrorOperation
	{
		ErrorOperationUnknown=0,
		ErrorOperationCreate=1,
		ErrorOperationDestroy=2,
		ErrorOperationOpen=3,
		ErrorOperationClose=4,
		ErrorOperationSwap=5,
		ErrorOperationRead=6,
		ErrorOperationWrite=7,
		ErrorOperationParse=8,
		ErrorOperationCommit=9,
		ErrorOperationConversion=10,
		ErrorOperationSelection=11
	};

	enum ErrorCause
	{
		ErrorCauseUnknown=0,
		ErrorCauseSystem=1,
		ErrorCauseOutOfMem=2,
		ErrorCauseAvailability=3,
		ErrorCauseSyntax=4,
		ErrorCauseInvalid=5,
		ErrorCauseNotAllowed=6,
		ErrorCauseInputFormat=7
	};

	struct ErrorCode
	{
		int value;

		ErrorCode( ErrorComponent comp_, ErrorOperation op_, ErrorCause cause_)
			:value( (int)comp_ * 10000 + (int)op_ * 100 + (int)cause_){}
		ErrorCode( int value_)
			:value(value_){}
		operator int() const			{return value;}

		ErrorComponent component() const		{return (ErrorComponent)((value / 10000) % 100);}
		ErrorOperation operation() const		{return (ErrorOperation)((value /   100) % 100);}
		ErrorCause cause() const			{return (ErrorCause)((value        ) % 100);}

		void setComponent( const ErrorComponent& comp_)	{if (comp_) value = value - ((value / 10000) % 100) + (int)comp_ * 10000;}
		void setOperation( const ErrorOperation& op_)	{if (op_) value = value -   ((value /   100) % 100) + (int)op_ *   100;}
		void setCause( const ErrorCause& cause_)	{if (cause_) value = value -   ((value     ) % 100) + (int)cause_;}
	};
}//namespace
#endif



