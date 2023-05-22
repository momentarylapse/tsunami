/*
 * HuiTimer.cpp
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#include "time.h"
#include "msg.h"
#include <thread>

namespace os {

static std::thread::id main_thread_id = std::this_thread::get_id();

bool is_main_thread() {
	return main_thread_id == std::this_thread::get_id();
}

void require_main_thread(const string &msg) {
	if (!is_main_thread()) {
		msg_error("called from non-main thread: " + msg);
	}
}




Timer::Timer() {
	prev_time = cur_time = std::chrono::high_resolution_clock::now();
}

void Timer::reset() {
	get();
}

float Timer::peek() {
	cur_time = std::chrono::high_resolution_clock::now();
	auto dt = std::chrono::duration_cast<std::chrono::duration<double>>(cur_time - prev_time);
	return dt.count();
}

float Timer::get() {
	float elapsed = peek();
	prev_time = cur_time;
	return elapsed;
}

//static bool _sleep_complained_ = false;

// don't call in main thread!!!!!
void sleep(float duration) {
	if (duration <= 0)
		return;
	std::this_thread::sleep_for(std::chrono::microseconds(int64(duration * 1000000)));

/*	if (main_thread_id == std::this_thread::get_id()) {
		if (!_sleep_complained_)
			msg_error("don't call hui::Sleep() in main thread!!!");
		_sleep_complained_ = true;
	}*/
}

}
