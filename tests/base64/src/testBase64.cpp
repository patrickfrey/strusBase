/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/base64.hpp"
#include "strus/lib/error.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>
#include <new>
#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace strus;

static void base64Test( bool verbose)
{
	static const char* input[] = {
		""
		,"M"
		,"Ma"
		,"Man"
		,"pleasure."
		,"leasure."
		,"easure."
		,"asure."
		,"sure."
		,"Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure."
		,"Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.."
		,"Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure..."
		,0
	};
	static const char* expected[] = {
		""
		,"TQ=="
		,"TWE="
		,"TWFu"
		,"cGxlYXN1cmUu"
		,"bGVhc3VyZS4="
		,"ZWFzdXJlLg=="
		,"YXN1cmUu"
		,"c3VyZS4="
		,"TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4="
		,"TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4u"
		,"TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4uLg=="
		,0
	};
	char const* const* ii = input;
	char const* const* ee = expected;
	for (unsigned int iidx=0; *ii; ++ii,++iidx,++ee)
	{
		if (verbose) std::cerr << "verifying encoding of '" << *ii << "' as '" << *ee << "'" << std::endl;
		strus::ErrorCode errcode = ErrorCodeUnknown;
		std::size_t inplen = std::strlen( *ii);
		std::size_t outlen = strus::base64EncodeLength( inplen);
		char* outbuf = (char*)std::malloc( outlen);
		if (!outbuf) throw std::bad_alloc();
		std::size_t enclen = strus::encodeBase64( outbuf, outlen, *ii, inplen, errcode);
		if (enclen == 0 && errcode) 
		{
			throw std::runtime_error( strus::errorCodeToString(errcode));
		}
		std::string outstr( outbuf, enclen);
		if (verbose) std::cerr << "got '" << outstr << "'" << std::endl;
		if (enclen != std::strlen(*ee))
		{
			throw strus::runtime_error("output length (%d) differs from expected (%d)", (int)enclen, (int)std::strlen(*ee));
		}
		if (0!=std::memcmp( *ee, outbuf, enclen))
		{
			throw strus::runtime_error("output (%s) differs from expected (%s)", outstr.c_str(), *ee);
		}
	}
}

int main( int argc, const char** argv)
{
	try
	{
		bool verbose = false;
		if (argc > 2) throw std::runtime_error( "too many arguments");
		if (argc == 2 && 0==std::strcmp( argv[1], "-V"))
		{
			verbose = true;
		}
		if (argc == 2 && 0==std::strcmp( argv[1], "-h"))
		{
			std::cerr << "Usage: testBase64 [-h,-V]" << std::endl;
			return 0;
		}
		std::cerr << "executing Base64 test" << std::endl;
		base64Test( verbose);
		std::cerr << "OK" << std::endl;
		return 0;
	}
	catch (const std::bad_alloc& err)
	{
		std::cerr << "ERROR " << err.what() << std::endl;
	}
	catch (const std::exception& err)
	{
		std::cerr << "ERROR " << err.what() << std::endl;
	}
	return -1;
}


