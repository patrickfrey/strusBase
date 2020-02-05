/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Variant type for fixed size numeric types (integers, floating point numbers)
/// \file numericVariant.hpp
#ifndef _STRUS_STORAGE_NUMERIC_VARIANT_TYPE_HPP_INCLUDED
#define _STRUS_STORAGE_NUMERIC_VARIANT_TYPE_HPP_INCLUDED
#include "strus/base/stdint.h"
#include <cstring>
#include <stdio.h>
#include <limits>

namespace strus {

/// \class NumericVariant
/// \brief Atomic type that can hold numeric values of different type
class NumericVariant
{
public:
	/// \brief Constructor from a signed integer
	/// \param[in] value value to assign to this numeric variant
	NumericVariant( int64_t value)
	{
		variant.Int = value;
		type = Int;
	}

	/// \brief Constructor from an unsigned integer
	/// \param[in] value value to assign to this numeric variant
	NumericVariant( uint64_t value)
	{
		variant.UInt = value;
		type = UInt;
	}

	/// \brief Constructor from a single precision floating point number
	/// \param[in] value value to assign to this numeric variant
	NumericVariant( double value)
	{
		variant.Float = value;
		type = Float;
	}
	/// \brief Constructor from a string
	/// \param[in] value value to parse and assign to this numeric variant
	NumericVariant( const char* val_)
	{
		if (!initFromString( val_)) init();
	}
	/// \brief Default constructor (as undefined value)
	NumericVariant()
	{
		init();
	}
	static NumericVariant asint( int64_t val)
	{
		return NumericVariant( val);
	}
	static NumericVariant asuint( uint64_t val)
	{
		return NumericVariant( val);
	}
	static NumericVariant asdouble( double val)
	{
		return NumericVariant( val);
	}
	/// \brief Copy constructor 
	/// \param[in] o numeric variant to copy
	NumericVariant( const NumericVariant& o)
	{
		type = o.type;
		variant = o.variant;
	}

	void init()
	{
		type = Null;
		variant.UInt = 0;
	}

	class String
	{
	public:
		String()
		{
			m_buf[0] = '\0';
		}

		explicit String( const NumericVariant& val, int precision=-1);

		operator const char*() const	{return m_buf;}
		const char* c_str() const	{return m_buf;}

	private:
		char m_buf[ 128];
	};

	String tostring( int precision=-1) const
	{
		return String( *this, precision);
	}

	/// \brief Initialize numeric variant parsed from a Ascii source string
	/// \param[in] src source string to parse
	/// \return bool true on success, false on failure
	bool initFromString( const char* src);

	/// \brief Find out if this value is defined
	/// \return true, if yes
	bool defined() const
	{
		return type != Null;
	}

	/// \brief Template for casting to a defined value type
	/// \tparam TYPE what type to cast this numeric variant to
	template <typename TYPE>
	TYPE cast() const
	{
		switch (type)
		{
			case Null: return TYPE();
			case Int: return (TYPE)variant.Int;
			case UInt: return (TYPE)variant.UInt;
			case Float: return (TYPE)variant.Float;
		}
		return TYPE();
	}

	/// \brief Cast to a single precision floating point number
	operator double() const
	{
		return cast<double>();
	}
	/// \brief Cast to a signed integer
	operator int64_t() const
	{
		return cast<int>();
	}
	/// \brief Cast to an unsigned integer
	operator uint64_t() const
	{
		return cast<uint64_t>();
	}

	/// \brief Cast to a signed integer
	int64_t toint() const
	{
		if (type == Float)
		{
			if (variant.Float < 0.0)
			{
				return (int64_t)(variant.Float - 2*std::numeric_limits<double>::epsilon());
			}
			else if (variant.Float > 0.0)
			{
				return (int64_t)(variant.Float + 2*std::numeric_limits<double>::epsilon());
			}
		}
		return cast<int64_t>();
	}

	/// \brief Cast to an unsigned integer
	uint64_t touint() const
	{
		if (type == Float)
		{
			if (variant.Float < 0.0)
			{
				return 0;
			}
			else if (variant.Float > 0.0)
			{
				return (uint64_t)(variant.Float + 2*std::numeric_limits<double>::epsilon());
			}
		}
		return cast<uint64_t>();
	}

	/// \brief Cast to an unsigned integer
	double tofloat() const
	{
		return cast<double>();
	}

	/// \brief Test for equality
	/// \param[in] o numeric variant to compare
	/// \return true, if yes
	bool operator == (const NumericVariant& o) const
	{
		return isequal(o);
	}
	/// \brief Test for inequality
	/// \param[in] o numeric variant to compare
	/// \return true, if yes
	bool operator != (const NumericVariant& o) const
	{
		return !isequal(o);
	}

	/// \brief Test for greater or equal
	/// \param[in] o numeric variant to compare
	/// \return true, if yes
	bool operator >= (const NumericVariant& o) const
	{
		return compare(o) >= 0;
	}

	/// \brief Test for greater
	/// \param[in] o numeric variant to compare
	/// \return true, if yes
	bool operator > (const NumericVariant& o) const
	{
		return compare(o) > 0;
	}

	/// \brief Test for smaller or equal
	/// \param[in] o numeric variant to compare
	/// \return true, if yes
	bool operator <= (const NumericVariant& o) const
	{
		return compare(o) <= 0;
	}

	/// \brief Test for smaller
	/// \param[in] o numeric variant to compare
	/// \return true, if yes
	bool operator < (const NumericVariant& o) const
	{
		return compare(o) < 0;
	}

	/// \brief Test for equality
	/// \param[in] o numeric variant to compare
	/// \return true, if yes
	bool isequal( const NumericVariant& o) const;

	/// \brief Comparison of numbers
	/// \param[in] o numeric variant to compare
	/// \return -1 <, +1 >, 0 =
	int compare( const NumericVariant& o) const;

	/// \brief Assignment operator for a singed integer
	/// \param[in] value value to assign to this numeric variant
	NumericVariant& operator=( int64_t value)
	{
		variant.Int = value;
		type = Int;
		return *this;
	}

	/// \brief Assignment operator for an unsinged integer
	/// \param[in] value value to assign to this numeric variant
	NumericVariant& operator=( uint64_t value)
	{
		variant.UInt = value;
		type = UInt;
		return *this;
	}

	/// \brief Assignment operator for a single precision floating point number
	/// \param[in] value value to assign to this numeric variant
	NumericVariant& operator=( double value)
	{
		variant.Float = value;
		type = Float;
		return *this;
	}

	/// \brief Assignment operator
	/// \param[in] o numeric variant to copy
	NumericVariant& operator=( const NumericVariant& o)
	{
		type = o.type;
		variant = o.variant;
		return *this;
	}

	/// \brief Enumeration of all types an numeric variant can have
	enum Type {
		Null,		///< uninitialized variant value
		Int,		///< signed integer number value
		UInt,		///< unsigned integer number value
		Float		///< floating point number value
	};
	typedef int64_t IntType;
	typedef uint64_t UIntType;
	typedef double FloatType;

	/// \brief Get the name of a type as string
	static const char* typeName( Type type);

	Type type;				///< Type of this numeric variant
	union
	{
		int64_t Int;			///< value in case of a signed integer
		uint64_t UInt;			///< value in case of an unsigned integer
		double Float;			///< value in case of a floating point number
	} variant;				///< Value of this numeric variant
};

}//namespace
#endif
