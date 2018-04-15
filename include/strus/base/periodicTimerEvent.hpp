/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Periodic timer event
#ifndef _STRUS_PERIODIC_TIMER_EVENT_HPP_INCLUDED
#define _STRUS_PERIODIC_TIMER_EVENT_HPP_INCLUDED
#include <stdexcept>

namespace strus {

/// \brief Periodic timer event
class PeriodicTimerEvent
{
public:
	explicit PeriodicTimerEvent( int secondsPeriod_)
		:m_secondsPeriod(secondsPeriod_)
	{
		if (!init()) throw std::bad_alloc();
	}
	virtual ~PeriodicTimerEvent()
	{
		clear();
	}

	bool start();
	void stop();

	/// \brief Event called
	virtual void tick(){}

private:
	void wait();
	bool init();
	void clear();

private:
	struct Data;
	Data* m_data;
	int m_secondsPeriod;
};

} // namespace
#endif
