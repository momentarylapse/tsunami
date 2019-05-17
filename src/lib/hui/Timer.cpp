/*
 * HuiTimer.cpp
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#include "Timer.h"
#include <thread>
static std::thread::id main_thread_id = std::this_thread::get_id();
void require_main_thread(const string &msg)
{
	if (main_thread_id != std::this_thread::get_id()){
		msg_error("called from non-main thread: " + msg);
	}

}


#ifdef OS_WINDOWS
	#include <direct.h>
	#include <tchar.h>
	#include <signal.h>
#endif
#ifdef OS_LINUX
	#include <sys/time.h>
	#include <unistd.h>
#endif


namespace hui
{


#ifdef OS_WINDOWS
	static bool HuiTimerPerfFlag = false;
	static float HuitTimerScal = 0;
#endif

void InitTimers()
{
#ifdef OS_WINDOWS
	LONGLONG perf_cnt;
	HuiTimerPerfFlag = (bool)QueryPerformanceFrequency((LARGE_INTEGER*)&perf_cnt);
	if (HuiTimerPerfFlag)
		HuitTimerScal = 1.0f / perf_cnt;
	else
		HuitTimerScal = 0.001f;
#endif
}

Timer::Timer()
{
	reset();
}

void Timer::reset()
{
	get();
}

float Timer::peek()
{
	float elapsed = 0;
	#ifdef OS_WINDOWS
		if (HuiTimerPerfFlag)
			QueryPerformanceCounter((LARGE_INTEGER *)&cur_time);
		else
			cur_time = timeGetTime();
		elapsed = (float)(cur_time - last_time) * HuitTimerScal;
	#endif
	#ifdef OS_LINUX
		gettimeofday(&cur_time, nullptr);
		elapsed = float(cur_time.tv_sec - last_time.tv_sec) + float(cur_time.tv_usec - last_time.tv_usec) * 0.000001f;
	#endif
	return elapsed;
}

float Timer::get()
{
	float elapsed = peek();
	last_time = cur_time;
	return elapsed;
}

static bool _sleep_complained_ = false;

// don't call in main thread!!!!!
void Sleep(float duration)
{
	if (duration <= 0)
		return;
/*	if (main_thread_id == std::this_thread::get_id()){
		if (!_sleep_complained_)
			msg_error("don't call hui::Sleep() in main thread!!!");
		_sleep_complained_ = true;
	}*/

#ifdef OS_WINDOWS
	Sleep((DWORD)(duration * 1000.0f));
#endif
#ifdef OS_LINUX
	usleep(duration * 1000000);
#endif
}

};


