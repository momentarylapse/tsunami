/*
 * util.h
 *
 *  Created on: 19 Aug 2023
 *      Author: michi
 */

#ifndef SRC_LIB_THREADS_UTIL_H_
#define SRC_LIB_THREADS_UTIL_H_

#include "../base/base.h"
#include <shared_mutex>

template<class T>
class InterThreadFifoBuffer {
	Array<T> buffer;
	std::shared_timed_mutex mtx;

public:
	void add(const T& t) {
		mtx.lock();
		buffer.add(t);
		mtx.unlock();
	}
	bool has_data() {
		mtx.lock();
		bool h = buffer.num > 0;
		mtx.unlock();
		return h;
	}
	int size() {
		mtx.lock();
		int s = buffer.num;
		mtx.unlock();
		return s;
	}
	T pop() {
		mtx.lock();
		T t{};
		if (buffer.num > 0) {
			t = buffer[0];
			buffer.erase(0);
		}
		mtx.unlock();
		return t;
	}
};


#endif /* SRC_LIB_THREADS_UTIL_H_ */
