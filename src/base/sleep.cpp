/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Sleep function
#include "strus/base/sleep.hpp"
#include "strus/base/dll_tags.hpp"

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace strus;

DLL_PUBLIC void strus::sleep( int nofsecs)
{
#ifdef _MSC_VER
	Sleep(pollingDelay);
#else
	::sleep( nofsecs);
#endif
}

