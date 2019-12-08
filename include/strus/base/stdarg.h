/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* \brief Portability issues for stdarg
*/
#ifndef _STRUS_BASE_STDARG_H_INCLUDED
#define _STRUS_BASE_STDARG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#include <cstdarg>
#else
#include <stdarg.h>
#endif

// Thanks to ocurnut for this hint for a va_copy for C++ < 11 (https://github.com/ocornut/imgui/issues/1085)
// On some platform vsnprintf() takes va_list by reference and modifies it.
// va_copy is the 'correct' way to copy a va_list but Visual Studio prior to 2013 doesn't have it.
#ifndef va_copy
#if defined(__GNUC__) || defined(__clang__)
#define va_copy(dest, src) __builtin_va_copy(dest, src)
#else
#define va_copy(dest, src) (dest = src)
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif

