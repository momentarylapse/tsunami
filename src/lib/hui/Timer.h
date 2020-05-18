/*
 * HuiTimer.h
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#ifndef HUITIMER_H_
#define HUITIMER_H_

#include "hui.h"
#include <chrono>

namespace hui {

class Timer {
public:
	Timer();
	float peek();
	float get();
	void reset();

private:
	std::chrono::high_resolution_clock::time_point prev_time;
	std::chrono::high_resolution_clock::time_point cur_time;
};


void _cdecl Sleep(float duration);

};

#endif /* HUITIMER_H_ */
