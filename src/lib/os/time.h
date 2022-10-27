/*
 * time.h
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#ifndef SRC_LIB_OS_TIME_H_
#define SRC_LIB_OS_TIME_H_

#include <chrono>

namespace os {

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


void sleep(float duration);

}

#endif /* SRC_LIB_OS_TIME_H_ */
