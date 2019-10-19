/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Shared reference of an object that is not shared between different threads.
/// \file "reference.hpp"
#ifndef _STRUS_REFERENCE_HPP_INCLUDED
#define _STRUS_REFERENCE_HPP_INCLUDED
#include <cstdlib>
#include <stdexcept>
#include <new>

namespace strus
{

template <typename Object>
class StandardReferenceDeleter
{
public:
	void operator()( Object* obj) {delete obj;}
};

/// \brief Shared pointer template with non thread-safe reference counting.
/// \note Similar to shared_ptr but without atomic (thread safe) reference counting
template <typename Object,class Deleter=StandardReferenceDeleter<Object> >
class Reference
{
public:
	/// \brief Default constructor
	Reference()
		:m_obj(0),m_refcnt(0){}
	/// \brief Constructor
	Reference( Object* obj_, bool doThrow=true)
		:m_obj(0),m_refcnt(0)
	{
		if (obj_) try
		{
			m_refcnt = newRefCnt();
			m_obj = obj_;
		}
		catch (const std::bad_alloc&)
		{
			Deleter()( obj_);
			if (doThrow) throw std::bad_alloc();
		}
	}
	/// \brief Copy constructor
	Reference( const Reference& o)
		:m_obj(o.m_obj),m_refcnt(o.m_refcnt)
	{
		if (m_refcnt) ++*m_refcnt;
	}

	/// \brief Destructor
	~Reference()
	{
		freeRef();
	}

	/// \brief Assignment operator
	Reference& operator = (const Reference& o)
	{
		freeRef();
		m_obj = o.m_obj;
		m_refcnt = o.m_refcnt;
		if (m_refcnt) ++*m_refcnt;
		return *this;
	}

	/// \brief Reinitialize the local value of the reference and dispose the old value if not referenced by others
	void reset( Object* obj_=0)
	{
		if (m_refcnt == NULL && obj_)
		{
			try
			{
				m_refcnt = newRefCnt();
				m_obj = obj_;
			}
			catch (const std::bad_alloc&)
			{
				Deleter()( obj_);
				return;
			}
		}
		else if (m_refcnt && *m_refcnt == 1 && obj_)
		{
			Deleter()( m_obj);
			m_obj = obj_;
		}
		else if (obj_ == NULL)
		{
			freeRef();
		}
		else
		{
			int* rc;
			try
			{
				rc = newRefCnt();
				freeRef();
				m_refcnt = rc;
				m_obj = obj_;
			}
			catch (const std::bad_alloc&)
			{
				Deleter()( obj_);
				throw std::bad_alloc();
			}
		}
	}

	/// \brief Object access operator
	Object* operator->()				{return m_obj;}
	/// \brief Object access operator
	const Object* operator->() const		{return m_obj;}
	/// \brief Object access operator
	Object& operator*()				{return *m_obj;}
	/// \brief Object access operator
	const Object& operator*() const			{return *m_obj;}

	/// \brief Object access as function
	const Object* get() const			{return m_obj;}
	/// \brief Object access as function
	Object* get()					{return m_obj;}

	/// \brief Release object reference and return the pointer to the object (with ownership) if this is possible
	/// \remark A multiply shared object reference cannot be released
	Object* release()
	{
		if (m_refcnt && *m_refcnt == 1)
		{
			--*m_refcnt;
			Object* rt = m_obj;
			std::free( m_refcnt);
			m_refcnt = 0;
			m_obj = 0;
			return rt;
		}
		else if (m_refcnt)
		{
			throw std::runtime_error( "cannot release shared object (having more than one reference)");
		}
		else
		{
			return 0;
		}
	}

	unsigned int refcnt() const
	{
		return m_refcnt?(*m_refcnt):0;
	}

	unsigned int use_count() const
	{
		return refcnt();
	}

	void check() const
	{
		if (m_refcnt && !m_obj)
		{
			throw std::runtime_error("bad state in reference");
		}
	}

private:
	static int* newRefCnt()
	{
		int* rt = (int*)std::malloc(sizeof(int));
		if (!rt) throw std::bad_alloc();
		*rt = 1;
		return rt;
	}
	void freeRef()
	{
		if (m_refcnt && --*m_refcnt == 0)
		{
			Deleter()( m_obj);
			std::free( m_refcnt);
		}
		m_refcnt = 0;
		m_obj = 0;
	}

private:
	Object* m_obj;
	mutable int* m_refcnt;
};

}
#endif

