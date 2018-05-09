/*
 * HuiTimer.h
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#ifndef HUITIMER_H_
#define HUITIMER_H_

#include "hui.h"

namespace hui
{

void InitTimers();

class Timer
{
public:
	Timer();
	float peek();
	float get();
	void reset();

private:
#ifdef OS_WINDOWS
	LONGLONG cur_time;
	LONGLONG last_time;
#else
	struct timeval cur_time;
	struct timeval last_time;
#endif
};


void _cdecl Sleep(float duration);

};

#endif /* HUITIMER_H_ */
