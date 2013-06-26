/*
 * HuiTimer.h
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#ifndef HUITIMER_H_
#define HUITIMER_H_

#include "hui.h"

class HuiTimer
{
public:
	HuiTimer();
	float peek();
	float get();
	void reset();

private:
#ifdef OS_WINDOWS
	LONGLONG CurTime;
	LONGLONG LastTime;
	float time_scale;
#endif
#ifdef OS_LINUX
	struct timeval CurTime, LastTime;
#endif
};


void _cdecl HuiSleep(float duration);

#endif /* HUITIMER_H_ */
