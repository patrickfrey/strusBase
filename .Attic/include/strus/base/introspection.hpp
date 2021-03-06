/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Helper classes and templates for introspection
/// \file introspection.hpp
#ifndef _STRUS_BASE_INTROSPECTION_HPP_INCLUDED
#define _STRUS_BASE_INTROSPECTION_HPP_INCLUDED
#include "strus/introspectionInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/numstring.hpp"
#include "strus/base/string_format.hpp"
#include <iostream>
#include <sstream>

/// \brief strus toplevel namespace
namespace strus
{

typedef IntrospectionInterface* (*IntrospectionConstructor)( const void* self, ErrorBufferInterface* errhnd);

char const* introspectionBadAllocError();

/// \class ConstIntrospection
/// \brief Introspection of a constant value
class ConstIntrospection
	:public IntrospectionInterface
{
public:
	ConstIntrospection( const char* val_, ErrorBufferInterface* errhnd_)
		:m_val(val_),m_errhnd(errhnd_){}
	virtual ~ConstIntrospection(){}

	virtual IntrospectionInterface* open( const std::string& name) const
	{
		return NULL;
	}
	virtual std::string value() const
	{
		try
		{
			return m_val?std::string(m_val):std::string();
		}
		catch (const std::bad_alloc&)
		{
			m_errhnd->report( ErrorCodeOutOfMem, "%s", strus::introspectionBadAllocError());
			return std::string();
		}
	}
	virtual std::vector<std::string> list() const
	{
		return std::vector<std::string>();
	}

private:
	const char* m_val;
	mutable ErrorBufferInterface* m_errhnd;
};

/// \class AtomicTypeIntrospection
/// \brief Template for building introspection of an atomic value
template <typename VALUE>
class AtomicTypeIntrospection
	:public IntrospectionInterface
{
public:
	AtomicTypeIntrospection( const VALUE* val_, ErrorBufferInterface* errhnd_)
		:m_val(val_),m_errhnd(errhnd_){}
	virtual ~AtomicTypeIntrospection(){}

	virtual IntrospectionInterface* open( const std::string& name) const
	{
		return NULL;
	}
	virtual std::string value() const
	{
		try
		{
			std::ostringstream out;
			out << *m_val;
			return out.str();
		}
		catch (const std::bad_alloc&)
		{
			m_errhnd->report( ErrorCodeOutOfMem, "%s", strus::introspectionBadAllocError());
			return std::string();
		}
	}
	virtual std::vector<std::string> list() const
	{
		return std::vector<std::string>();
	}

	static IntrospectionInterface* constructor( const void* self, ErrorBufferInterface* errhnd)
	{
		try
		{
			return new AtomicTypeIntrospection<VALUE>( (VALUE*)self, errhnd);
		}
		catch (const std::bad_alloc&)
		{
			errhnd->report( ErrorCodeOutOfMem, "%s", strus::introspectionBadAllocError());
			return NULL;
		}
	}

private:
	const VALUE* m_val;
	mutable ErrorBufferInterface* m_errhnd;
};

/// \class MapTypeIntrospection
/// \brief Template for building introspection of a std::map like structure with string as key
template <typename VALUE>
class MapTypeIntrospection
	:public IntrospectionInterface
{
public:
	MapTypeIntrospection( const VALUE* val_, ErrorBufferInterface* errhnd_, IntrospectionConstructor elementConstructor_)
		:m_val(val_),m_elementConstructor(elementConstructor_),m_errhnd(errhnd_){}

	virtual ~MapTypeIntrospection(){}

	virtual IntrospectionInterface* open( const std::string& name) const
	{
		try
		{
			typename VALUE::const_iterator mi = m_val->find( name);
			if (mi == m_val->end()) return NULL;
			return (*m_elementConstructor)( &mi->second, m_errhnd);
		}
		catch (const std::bad_alloc&)
		{
			m_errhnd->report( ErrorCodeOutOfMem, "%s", strus::introspectionBadAllocError());
			return NULL;
		}
	}
	virtual std::string value() const
	{
		return std::string();
	}
	virtual std::vector<std::string> list() const
	{
		try
		{
			std::vector<std::string> rt;
			typename VALUE::const_iterator mi = m_val->begin(), me = m_val->end();
			for (; mi != me; ++mi) rt.push_back( mi->first);
			return rt;
		}
		catch (const std::bad_alloc&)
		{
			m_errhnd->report( ErrorCodeOutOfMem, "%s", strus::introspectionBadAllocError());
			return std::vector<std::string>();
		}
	}

	static IntrospectionInterface* constructor( const void* self, ErrorBufferInterface* errhnd, IntrospectionConstructor elementConstructor)
	{
		try
		{
			return new MapTypeIntrospection<VALUE>( (VALUE*)self, errhnd, elementConstructor);
		}
		catch (const std::bad_alloc&)
		{
			errhnd->report( ErrorCodeOutOfMem, "%s", strus::introspectionBadAllocError());
			return NULL;
		}
	}

private:
	const VALUE* m_val;
	IntrospectionConstructor m_elementConstructor;
	mutable ErrorBufferInterface* m_errhnd;
};

/// \class MapTypeIntrospection
/// \brief Template for building introspection of a std::map like structure with string as key and an atomic as value
template <typename VALUE>
class AtomicMapTypeIntrospection
	:public MapTypeIntrospection<VALUE>
{
public:
	AtomicMapTypeIntrospection( const VALUE* val_, ErrorBufferInterface* errhnd_)
		:MapTypeIntrospection<VALUE>(val_,errhnd_,&AtomicTypeIntrospection<typename VALUE::mapped_type>::constructor){}
	virtual ~AtomicMapTypeIntrospection(){}

	static IntrospectionInterface* constructor( const void* self, ErrorBufferInterface* errhnd)
	{
		IntrospectionConstructor elementConstructor = &AtomicTypeIntrospection<typename VALUE::mapped_type>::constructor;
		return MapTypeIntrospection<VALUE>::constructor( self, errhnd, elementConstructor);
	}
};

/// \class VectorTypeIntrospection
/// \brief Template for building introspection of a std::vector like structure with the element index string as key
template <typename VALUE>
class VectorTypeIntrospection
	:public IntrospectionInterface
{
public:
	explicit VectorTypeIntrospection( const VALUE* val_, ErrorBufferInterface* errhnd_, IntrospectionConstructor elementConstructor_)
		:m_val(val_),m_elementConstructor(elementConstructor_),m_errhnd(errhnd_){}
	virtual ~VectorTypeIntrospection(){}

	virtual IntrospectionInterface* open( const std::string& name) const
	{
		try
		{
			NumParseError err = NumParseOk;
			std::size_t idx = strus::uintFromString( name.c_str(), name.size(), m_val->size(), err);
			return (*m_elementConstructor)( &(*m_val)[ idx], m_errhnd);
		}
		catch (const std::bad_alloc&)
		{
			m_errhnd->report( ErrorCodeOutOfMem, "%s", strus::introspectionBadAllocError());
			return NULL;
		}
	}
	virtual std::string value() const
	{
		return std::string();
	}
	virtual std::vector<std::string> list() const
	{
		try
		{
			std::vector<std::string> rt;
			int ii = 0, ie = m_val->size();
			for (; ii != ie; ++ii)
			{
				rt.push_back( strus::string_format( "%d", ii));
				if (rt.back().empty()) throw std::bad_alloc();
			}
			return rt;
		}
		catch (const std::bad_alloc&)
		{
			m_errhnd->report( ErrorCodeOutOfMem, "%s", strus::introspectionBadAllocError());
			return std::vector<std::string>();
		}
	}

	static IntrospectionInterface* constructor( const void* self, ErrorBufferInterface* errhnd, IntrospectionConstructor elementConstructor_)
	{
		try
		{
			return new VectorTypeIntrospection<VALUE>( (VALUE*)self, errhnd, elementConstructor_);
		}
		catch (const std::bad_alloc&)
		{
			errhnd->report( ErrorCodeOutOfMem, "%s", strus::introspectionBadAllocError());
			return NULL;
		}
	}

private:
	const VALUE* m_val;
	IntrospectionConstructor m_elementConstructor;
	mutable ErrorBufferInterface* m_errhnd;
};

/// \brief Template for building introspection of a std::vector like structure with the element index string as key and an atomic as value
template <typename VALUE>
class AtomicVectorTypeIntrospection
	:public VectorTypeIntrospection<VALUE>
{
public:
	explicit AtomicVectorTypeIntrospection( const VALUE* val_, ErrorBufferInterface* errhnd_)
		:VectorTypeIntrospection<VALUE>(val_,errhnd_,&AtomicTypeIntrospection<typename VALUE::value_type>::constructor){}
	virtual ~AtomicVectorTypeIntrospection(){}

	static IntrospectionInterface* constructor( const void* self, ErrorBufferInterface* errhnd)
	{
		IntrospectionConstructor elementConstructor = &AtomicTypeIntrospection<typename VALUE::value_type>::constructor;
		return VectorTypeIntrospection<VALUE>::constructor( self, errhnd, elementConstructor);
	}
};

typedef AtomicVectorTypeIntrospection<std::vector<std::string> > StringVectorIntrospection;

struct ElementDescription
{
	const char* name;
	int ofs;
	IntrospectionConstructor constructor;

	ElementDescription( const char* name_, int ofs_, IntrospectionConstructor constructor_)
		:name(name_),ofs(ofs_),constructor(constructor_){}
	ElementDescription( const ElementDescription& o)
		:name(o.name),ofs(o.ofs),constructor(o.constructor){}
};

template <typename STRUCT>
class StructTypeIntrospectionDescription
{
public:
	explicit StructTypeIntrospectionDescription()
		:m_name(0){}

	StructTypeIntrospectionDescription& operator[]( const char* value)
	{
		m_name = value;
		return *this;
	}

	template <typename ELEMENT>
	StructTypeIntrospectionDescription& operator()( const char* name, ELEMENT STRUCT::*memb, IntrospectionConstructor constructor)
	{
		const STRUCT* base = 0;
		int ofs = (const char*)&(base->*memb) - (const char*)base;
		m_ar.push_back( ElementDescription( name, ofs, constructor));
		return *this;
	}

	IntrospectionInterface* open( const STRUCT* st, const std::string& name, ErrorBufferInterface* errhnd) const
	{
		if (m_name && name == "name") return new ConstIntrospection( m_name, errhnd);
		std::vector<ElementDescription>::const_iterator ai = m_ar.begin(), ae = m_ar.end();
		for (; ai != ae; ++ai)
		{
			if (name == ai->name)
			{
				return ai->constructor( (const void*)((const char*)st + ai->ofs), errhnd);
			}
		}
		return NULL;
	}

	std::vector<std::string> list() const
	{
		std::vector<std::string> rt;
		if (m_name) rt.push_back( "name");
		std::vector<ElementDescription>::const_iterator ai = m_ar.begin(), ae = m_ar.end();
		for (; ai != ae; ++ai) rt.push_back( ai->name);
		return rt;
	}
private:
	const char* m_name;
	std::vector<ElementDescription> m_ar;
};

/// \class StructTypeIntrospection
/// \brief Template for building introspection of a structure
template <typename STRUCT>
class StructTypeIntrospection
	:public IntrospectionInterface
{
public:
	typedef StructTypeIntrospectionDescription<STRUCT> Description;

	StructTypeIntrospection( const STRUCT* st_, const Description* descr_, ErrorBufferInterface* errhnd_)
		:m_errhnd(errhnd_),m_st(st_),m_descr(descr_){}

	/// \brief Destructor
	virtual ~StructTypeIntrospection(){}

	virtual IntrospectionInterface* open( const std::string& name) const
	{
		try
		{
			return m_descr->open( m_st, name, m_errhnd);
		}
		catch (const std::bad_alloc&)
		{
			m_errhnd->report( ErrorCodeOutOfMem, "%s", strus::introspectionBadAllocError());
			return NULL;
		}
	}

	virtual std::string value() const
	{
		return std::string();
	}

	virtual std::vector<std::string> list() const
	{
		try
		{
			return m_descr->list();
		}
		catch (const std::bad_alloc&)
		{
			m_errhnd->report( ErrorCodeOutOfMem, "%s", strus::introspectionBadAllocError());
			return std::vector<std::string>();
		}
	}

private:
	mutable ErrorBufferInterface* m_errhnd;
	const STRUCT* m_st;
	const Description* m_descr;
};

std::vector<std::string> getIntrospectionElementList( const char** ar, class ErrorBufferInterface* errorhnd);

std::string introspection_tostring( const IntrospectionInterface* introspection, int maxdepth);

}//namespace
#endif

