/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/programLexer.hpp"
#include "strus/base/localErrorBuffer.hpp"
#include "strus/base/string_format.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>

using namespace strus;

#define STRUS_LOWLEVEL_DEBUG

static LocalErrorBuffer g_errhnd;

const char* g_testProgram_Pascal =
"PROGRAM( input, output)\n"
"VAR cachesize := 32K;\n"
"VAR name := \"example string\";\n"
"// Not really an old Pascal comment, but available in Free Pascal"
"PROCEDURE calcCircularArea( VAR radius :FLOAT)\n"
"// This procedure calculates the area of a circle from its radius"
"BEGIN\n"
"    calcCircularArea := radius * radius * 3.14159265359;\n"
"    // Return the area of the circle"
"END\n"
"\n"
"BEGIN\n"
"    writeln( output, calcCircularArea( 2 * 2.7));\n"
"END\n"
;

enum Tokens_Pascal {
	TokPROGRAM,
	TokVAR,
	TokPROCEDURE,
	TokBEGIN,
	TokEND,
	TokFLOAT,
	TokINT,
	TokIdentifier,
	TokFloat,
	TokDim,
	TokInteger,
	TokOpenOvalBracket,
	TokCloseOvalBracket,
	TokOpenSquareBracket,
	TokCloseSquareBracket,
	TokAssign,
	TokComma,
	TokColon,
	TokSemiColon,
	TokOpMult,
	TokOpPlus,
	TokOpMinus,
	TokPath
};
static const char* g_tokens_Pascal[] = {
	"PROGRAM",
	"VAR",
	"PROCEDURE",
	"BEGIN",
	"END",
	"FLOAT",
	"INT",
	"[a-zA-Z_][a-zA-Z0-9_]*",
	"[+-]*[0-9][0-9_]*[.][0-9]*",
	"[+-]*[0-9][0-9_]*[KMGTP]",
	"[+-]*[0-9][0-9_]*",
	"\\(",
	"\\)",
	"\\[",
	"\\]",
	":\\=",
	",",
	":",
	";",
	"\\*",
	"\\+",
	"\\-",
	"[/][^\\;,\\{]*",
	NULL
};
static const char* g_errtokens_Pascal[] = {
	"[0-9][0-9]*[a-zA-Z_][a-zA-Z_]*",
	NULL
};
static const char* g_eolncomment_Pascal = "//";

static const char* g_expected_Pascal =
	"token 'PROGRAM': \"PROGRAM\"\n"
	"token '\\(': \"(\"\n"
	"token '[a-zA-Z_][a-zA-Z0-9_]*': \"input\"\n"
	"token ',': \",\"\n"
	"token '[a-zA-Z_][a-zA-Z0-9_]*': \"output\"\n"
	"token '\\)': \")\"\n"
	"token 'VAR': \"VAR\"\n"
	"token '[a-zA-Z_][a-zA-Z0-9_]*': \"cachesize\"\n"
	"token ':\\=': \":=\"\n"
	"token '[+-]*[0-9][0-9_]*[KMGTP]': \"32K\"\n"
	"token ';': \";\"\n"
	"token 'VAR': \"VAR\"\n"
	"token '[a-zA-Z_][a-zA-Z0-9_]*': \"name\"\n"
	"token ':\\=': \":=\"\n"
	"string 'example string'\n"
	"token ';': \";\"\n"
	"token '[a-zA-Z_][a-zA-Z0-9_]*': \"calcCircularArea\"\n"
	"token ':\\=': \":=\"\n"
	"token '[a-zA-Z_][a-zA-Z0-9_]*': \"radius\"\n"
	"token '\\*': \"*\"\n"
	"token '[a-zA-Z_][a-zA-Z0-9_]*': \"radius\"\n"
	"token '\\*': \"*\"\n"
	"token '[+-]*[0-9][0-9_]*[.][0-9]*': \"3.14159265359\"\n"
	"token ';': \";\"\n"
	"token 'BEGIN': \"BEGIN\"\n"
	"token '[a-zA-Z_][a-zA-Z0-9_]*': \"writeln\"\n"
	"token '\\(': \"(\"\n"
	"token '[a-zA-Z_][a-zA-Z0-9_]*': \"output\"\n"
	"token ',': \",\"\n"
	"token '[a-zA-Z_][a-zA-Z0-9_]*': \"calcCircularArea\"\n"
	"token '\\(': \"(\"\n"
	"token '[+-]*[0-9][0-9_]*': \"2\"\n"
	"token '\\*': \"*\"\n"
	"token '[+-]*[0-9][0-9_]*[.][0-9]*': \"2.7\"\n"
	"token '\\)': \")\"\n"
	"token '\\)': \")\"\n"
	"token ';': \";\"\n"
	"token 'END': \"END\"\n"
;

const char* diffpos( char const* ri, char const* ei)
{
	while (*ei == *ri) {++ri;++ei;}
	return ri;
}

void testExample_Pascal()
{
	ProgramLexer lexer( g_testProgram_Pascal, g_eolncomment_Pascal, g_tokens_Pascal, g_errtokens_Pascal, &g_errhnd);
	std::ostringstream out;
	while (!lexer.next().end())
	{
		ProgramLexem cur = lexer.current();
		if (cur.isString())
		{
			out << "string '" << cur.value() << "'" << std::endl;
		}
		else if (cur.isToken())
		{
			out << "token '" << g_tokens_Pascal[ cur.id()] << "': \"" << cur.value() << "\"" << std::endl;
		}
		else
		{
			throw std::runtime_error( strus::string_format( "unknown token on line %d of input: %s", lexer.lineno(), cur.value().c_str()));
		}
	}
	if (g_errhnd.hasError())
	{
		std::string location = lexer.currentLocationString( -30, 50, "<!>");
		throw std::runtime_error( strus::string_format( "error on line %d of input: %s (location: %s)", lexer.lineno(), g_errhnd.fetchError(), location.c_str()));
	}
	std::string outstr = out.str();
	if (0!=std::strcmp( outstr.c_str(), g_expected_Pascal))
	{
		std::cout << "OUTPUT:\n" << outstr << std::endl;
		std::cerr << "EXPECTED:\n" << g_expected_Pascal << std::endl;
		throw std::runtime_error( strus::string_format( "output is not as expected at: %.30s", diffpos( outstr.c_str(), g_expected_Pascal)));
	}
}


int main( int argc, const char* argv[])
{
	if (argc >= 2 && (std::strcmp( argv[1], "-h") == 0 || std::strcmp( argv[1], "--help") == 0))
	{
		std::cerr << "testRegex" << std::endl;
		return 0;
	}
	try
	{
		testExample_Pascal();
		std::cerr << "OK" << std::endl;
		return 0;
	}
	catch (const std::runtime_error& err)
	{
		std::cerr << std::endl << "ERROR " << err.what() << std::endl;
		return -1;
	}
	catch (const std::bad_alloc& )
	{
		std::cerr << std::endl << "ERROR out of memory" << std::endl;
		return -2;
	}
	catch (...)
	{
		std::cerr << std::endl << "EXCEPTION uncaught" << std::endl;
		return -3;
	}
}

