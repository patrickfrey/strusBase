/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Datastructure for introspection of structures
/// \file structView.hpp
#ifndef _STRUS_BASE_INTROSPECTION_HPP_INCLUDED
#define _STRUS_BASE_INTROSPECTION_HPP_INCLUDED
#include "strus/numericVariant.hpp"
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <stdexcept>
#include <utility>
#include <iostream>
#include <sstream>

/// \brief strus toplevel namespace
namespace strus
{

/// \class Introspection
/// \brief Datastructure for introspection
class StructView
{
public:
	enum {
		FloatPrecisionToString=6,
	};
	enum Type {
		Null,
		String,
		Numeric,
		Structure
	};
	/// \brief Destructor
	~StructView(){}
	/// \brief Copy constructor
	StructView( const StructView& o)
		:m_type(o.m_type),m_string(o.m_string),m_numeric(o.m_numeric),m_ar(o.m_ar),m_dict(o.m_dict){}

	/// \brief Default constructor
	StructView()
		:m_type(Null){}
	/// \brief Constructor
	StructView( int value)
		:m_type(Numeric),m_numeric( NumericVariant::asint( value)){}
	/// \brief Constructor
	StructView( unsigned int value)
		:m_type(Numeric),m_numeric( NumericVariant::asuint( value)){}
	/// \brief Constructor
	StructView( double value)
		:m_type(Numeric),m_numeric( NumericVariant::asdouble( value)){}
	/// \brief Constructor
	StructView( const char* value)
		:m_type(String),m_string( value ? value : ""){}
	/// \brief Constructor
	StructView( const std::string& value)
		:m_type(String),m_string( value){}
	/// \brief Constructor
	StructView( const NumericVariant& value)
		:m_type(Numeric),m_numeric( value){}
	/// \brief Constructor
	StructView( const std::vector<std::string>& init)
		:m_type(Structure),m_ar( init.begin(), init.end()){}

	/// \brief Assignment operator
	StructView& operator=( const StructView& o)
		{m_type=o.m_type; m_string=o.m_string; m_numeric=o.m_numeric; m_ar=o.m_ar; m_dict=o.m_dict; return *this;}

#if __cplusplus >= 201103L
	StructView( StructView&& o)
		:m_type(o.m_type),m_string(std::move(o.m_string)),m_numeric(std::move(o.m_numeric)),m_ar(std::move(o.m_ar)),m_dict(std::move(o.m_dict)){}
	StructView& operator=( StructView&& o)
		{m_type=o.m_type; m_string=std::move(o.m_string); m_numeric=std::move(o.m_numeric); m_ar=std::move(o.m_ar); m_dict=std::move(o.m_dict); return *this;}

	StructView( const std::initializer_list<StructView>& init)
		:m_type(Structure),m_ar( init.begin(), init.end()){}

	StructView( const std::initializer_list<std::pair<const char*,StructView> >& init)
		:m_type(Structure)
	{
		std::initializer_list<std::pair<const char*,StructView> >::const_iterator ii = init.begin(), ie = init.end();
		for (; ii != ie; ++ii) m_dict[ ii->first] = ii->second;
	}
#endif

	StructView& operator()( const std::string& name, const StructView& value)
	{
		if (m_type == Null || (m_type == Structure && m_ar.empty()))
		{
			m_type = Structure;
			m_dict[ name] = value;
			return *this;
		}
		else throw std::runtime_error( "invalid StructView definition");
	}
	StructView& operator()( const std::pair<std::string,std::string>& keyValuePair)
	{
		if (m_type == Null || (m_type == Structure && m_ar.empty()))
		{
			m_type = Structure;
			m_dict[ keyValuePair.first] = keyValuePair.second;
			return *this;
		}
		else throw std::runtime_error( "invalid StructView definition");
	}
	StructView& operator()( const char* name, const StructView& value)
	{
		if (m_type == Null || (m_type == Structure && m_ar.empty()))
		{
			m_type = Structure;
			m_dict[ name] = value;
			return *this;
		}
		else throw std::runtime_error( "invalid StructView definition");
	}
	StructView& operator()( const StructView& value)
	{
		if (m_type == Null || (m_type == Structure && m_dict.empty()))
		{
			m_type = Structure;
			m_ar.push_back( value);
			return *this;
		}
		else throw std::runtime_error( "invalid StructView definition");
	}

	/// \brief Get the type of the element
	Type type() const
	{
		return m_type;
	}
	std::string tostring() const
	{
		std::string rt;
		if (!appendToString( rt)) throw std::bad_alloc();
		return rt;
	}
	const std::string& asstring() const
	{
		return m_string;
	}
	const NumericVariant& asnumeric() const
	{
		return m_numeric;
	}
	bool defined() const
	{
		return (m_type != Null);
	}
	bool isAtomic() const
	{
		return (m_type != Structure);
	}
	bool isArray() const
	{
		return (m_type == Structure && !m_ar.empty());
	}
	std::size_t arraySize() const
	{
		return m_ar.size();
	}
	std::size_t dictSize() const
	{
		return m_dict.size();
	}
	const StructView* get( std::size_t idx) const
	{
		return &m_ar[ idx];
	}
	StructView* get( std::size_t idx)
	{
		return &m_ar[ idx];
	}
	const StructView* get( const std::string& key) const
	{
		std::map<std::string,StructView>::const_iterator di = m_dict.find( key);
		return di == m_dict.end() ? NULL : &di->second;
	}
	StructView* get( const std::string& key)
	{
		std::map<std::string,StructView>::iterator di = m_dict.find( key);
		return di == m_dict.end() ? NULL : &di->second;
	}
	typedef std::map<std::string,StructView>::const_iterator dict_iterator;
	dict_iterator dict_begin() const	{return m_dict.begin();}
	dict_iterator dict_end() const	{return m_dict.end();}

	typedef std::vector<StructView>::const_iterator array_iterator;
	array_iterator array_begin() const	{return m_ar.begin();}
	array_iterator array_end() const	{return m_ar.end();}

	template<typename Element>
	static StructView getArray( const std::vector<Element>& init)
	{
		StructView rt;
		typename std::vector<Element>::const_iterator ei = init.begin(), ee = init.end();
		for (; ei != ee; ++ei) {rt( *ei);}
		return rt;
	}

private:
	bool appendToString( std::string& buf) const;

private:
	Type m_type;
	std::string m_string;
	NumericVariant m_numeric;
	std::vector<StructView> m_ar;
	std::map<std::string,StructView> m_dict;
};

}//namespace
#endif

