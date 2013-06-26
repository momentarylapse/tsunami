/*
 * HuiTimer.cpp
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#include "HuiTimer.h"


#ifdef OS_WINDOWS
	#include <direct.h>
	#include <tchar.h>
	#include <signal.h>
#endif
#ifdef OS_LINUX
	#include <sys/time.h>
	#include <unistd.h>
#endif



#ifdef OS_WINDOWS
	extern LONGLONG perf_cnt;
	extern bool perf_flag;
	extern float time_scale;
#endif

HuiTimer::HuiTimer()
{
	reset();
}

void HuiTimer::reset()
{
	get();
}

float HuiTimer::peek()
{
	float elapsed = 0;
	#ifdef OS_WINDOWS
		if (perf_flag)
			QueryPerformanceCounter((LARGE_INTEGER *)&CurTime);
		else
			CurTime = timeGetTime();
		elapsed = (CurTime - LastTime) * time_scale;
	#endif
	#ifdef OS_LINUX
		gettimeofday(&CurTime,NULL);
		elapsed = float(CurTime.tv_sec - LastTime.tv_sec) + float(CurTime.tv_usec - LastTime.tv_usec) * 0.000001f;
	#endif
	return elapsed;
}

float HuiTimer::get()
{
	float elapsed = peek();
#ifdef OS_WINDOWS
	LastTime = CurTime;
#endif
#ifdef OS_LINUX
	LastTime = CurTime;
#endif
	return elapsed;
}




void HuiSleep(float duration)
{
	if (duration <= 0)
		return;
#ifdef OS_WINDOWS
	Sleep(duration * 1000);
#endif
#ifdef OS_LINUX
	usleep(duration * 1000000);
#endif
}




