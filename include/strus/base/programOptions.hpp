/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_BASE_PROGRAM_OPTIONS_HPP_INCLUDED
#define _STRUS_BASE_PROGRAM_OPTIONS_HPP_INCLUDED
#include "strus/errorBufferInterface.hpp"
#include <map>
#include <vector>
#include <string>
#include <iostream>

namespace strus {

/// \brief Program options parser
class ProgramOptions
{
public:
	/// \brief Default constructor
	ProgramOptions( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_argc(0),m_argv(0){}

	/// \brief Copy constructor
	ProgramOptions( const ProgramOptions& o)
		:m_errorhnd(o.m_errorhnd),m_argc(o.m_argc),m_argv(o.m_argv),m_opt(o.m_opt){}

	/// \brief Constructor
	ProgramOptions( ErrorBufferInterface* errorhnd_, int argc_, const char** argv_, int nofopt, ...);

	/// \brief Test if an option specified by its long name is defined
	bool operator()( const std::string& optname) const;

	/// \brief Test if an option specified by its long name is defined
	bool operator()( const char* optname) const;

	/// \brief Get the value of an argument specified by its index
	/// \param[in] index of the argument starting with 0
	/// \note first argument (not the program name!) has index 0
	/// \return the argument value as string or NULL on failure
	const char* operator[]( std::size_t idx) const;

	/// \brief Get the value of an option specified by its long name
	/// \param[in] optname long name of the option
	/// \return the option value as string or NULL on failure
	const char* operator[]( const std::string& optname) const;

	/// \brief Get the value of an option specified by its long name as signed integer
	/// \param[in] optname long name of the option
	/// \return the option value as integer or 0 on failure
	int asInt( const std::string& optname) const;

	/// \brief Get the value of an option specified by its long name as unsigned integer
	/// \param[in] optname long name of the option
	/// \return the option value as integer or 0 on failure
	unsigned int asUint( const std::string& optname) const;

	/// \brief Get the value of an option specified by its long name as unsigned integer
	/// \param[in] optname long name of the option
	/// \return the option value as integer or 0 on failure
	double asDouble( const std::string& optname) const;

	/// \brief Get the list of values of an option specified by its long name
	/// \param[in] optname long name of the option
	/// \return the option values as vector, empty on failure
	std::vector<std::string> list( const std::string& optname) const;

	/// \brief Get the number of program arguments
	/// \return the number of program arguments without options and the program name
	int nofargs() const;

	/// \brief Get the list of program arguments
	/// \return the list of program arguments without options and the program name
	const char** argv() const;

	/// \brief Print all proram options defined
	/// \param[in] where to print the options to
	void print( std::ostream& out);

	std::pair<const char*,const char*> conflictingOpts( int n, ...);

private:
	enum OptType {SingleBool,RepeatingBool, WithArgument};

	/// \brief Internal program options name map
	class OptMapDef
	{
	public:
		explicit OptMapDef( ErrorBufferInterface* errorhnd_) :m_errorhnd(errorhnd_){}

		void add( const char* arg);

		bool getOpt( const char* argv, std::vector<std::string>& optlist, std::string& optarg);

		ErrorBufferInterface* m_errorhnd;
		std::map<std::string,OptType> m_longnamemap;
		std::map<char,std::string> m_aliasmap;
	};

private:
	ErrorBufferInterface* m_errorhnd;				///< error buffer for reporting errors and exceptions
	std::size_t m_argc;						///< number of arguments without program and options
	char const** m_argv;						///< array of arguments without program and options
	typedef std::multimap<std::string,std::string> OptMap;
	typedef std::pair<std::string,std::string> OptMapElem;
	OptMap m_opt;							///< options map
};

}//namespace
#endif

