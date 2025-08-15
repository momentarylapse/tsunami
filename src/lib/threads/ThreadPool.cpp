//
// Created by michi on 7/21/25.
//

#include "ThreadPool.h"
#include <atomic>
#include <unistd.h>
#include "../os/msg.h"
#if __has_include("../profiler/Profiler.h")
#include "../profiler/Profiler.h"
//#define USE_PROFILER
#endif

class PoolWorkerThread : public Thread {
public:
	explicit PoolWorkerThread(int _id) {
		id = _id;
#ifdef USE_PROFILER
		profiler::create_channel(format("thread%d", id));
#endif
	}
	int id;
	int channel = -1;
	std::atomic<bool> awake = false;
	std::atomic<bool> has_work = false;
	std::function<void()> f;
	void on_run() override {
		while (true) {
			if (has_work) {
#ifdef USE_PROFILER
				profiler::begin(channel);
#endif
				f();
#ifdef USE_PROFILER
				profiler::end(channel);
#endif
				has_work = false;
			}
			cancelation_point();
			if (awake)
				usleep(10);
			else
				usleep(100);
		}

	}
};

ThreadPool::ThreadPool(int num_threads) {
	if (num_threads < 0)
		num_threads = max(Thread::get_num_cores() - 1, 1);
	for (int i=0; i<num_threads; i++) {
		threads.add(new PoolWorkerThread(i));
	}
}

ThreadPool::~ThreadPool() {
	for (auto t: threads)
		delete t;
}

void ThreadPool::_begin() {
	for (auto t: threads) {
		if (!t->running)
			t->run();
	}
}

void ThreadPool::_end() {

	// wait till finished
	bool any_running = true;
	while (any_running) {
		any_running = false;
		for (auto t: threads)
			if (t->has_work)
				any_running = true;
	}

	for (auto t: threads)
		t->awake = false;
	//t->join();
}



void ThreadPool::_emit_cluster(std::function<void()> f) {
	while (true) {
		for (auto t: threads)
			if (!t->has_work) {
				t->f = f;
				t->has_work = true;
				return;
			}
	}
}


