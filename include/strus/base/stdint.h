/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* \brief Definition of integer types
*/
#ifndef _STRUS_BASE_STDINT_H_INCLUDED
#define _STRUS_BASE_STDINT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#error stdint definitions missing for Windows
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
}
#endif
#endif
