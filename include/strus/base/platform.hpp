/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Retrieve Platform dependent constants
#ifndef _STRUS_PLATFORM_HPP_INCLUDED
#define _STRUS_PLATFORM_HPP_INCLUDED

namespace strus {
namespace platform {
	enum {CacheLineSize=64};
	//... TODO: Probing of this constant

	int cores();
}}//namespace

#endif

