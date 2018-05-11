/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for introspection
/// \file introspectionInterface.hpp
#ifndef _STRUS_BASE_INTROSPECTION_INTERFACE_HPP_INCLUDED
#define _STRUS_BASE_INTROSPECTION_INTERFACE_HPP_INCLUDED
#include "strus/errorCodes.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;

/// \class IntrospectionInterface
/// \brief Interface for introspection
class IntrospectionInterface
{
public:
	/// \brief Destructor
	virtual ~IntrospectionInterface(){}

	/// \brief Open a new deeper level of introspection
	/// \param[in] name name of the element to open
	/// \return an introspection interface (with ownership)
	virtual IntrospectionInterface* open( const std::string& name) const=0;

	/// \brief Get the element Value
	/// \return the value as string
	virtual std::string value() const=0;

	/// \brief Get the list of elements
	/// \return the list of element names that can be opened
	virtual std::vector<std::string> list() const=0;
};

}//namespace
#endif

