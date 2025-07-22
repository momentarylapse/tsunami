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
	std::atomic<int> task_id_begin = -1;
	std::atomic<int> task_id_end = -1;
	std::function<void(int)> f;
	void on_run() override {
		while (true) {
			if (f and task_id_begin >= 0) {
#ifdef USE_PROFILER
				profiler::begin(channel);
#endif
				for (int i=task_id_begin; i<task_id_end; i++)
					f(i);
#ifdef USE_PROFILER
				profiler::end(channel);
#endif
				task_id_begin = -1;
			}
			cancelation_point();
			if (f)
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

void ThreadPool::run(int n, std::function<void(int)> f, int cluster_size) {
	for (auto t: threads) {
		t->f = f;
		if (!t->running)
			t->run();
	}

	for (int i = 0; i < n; i+=cluster_size) {
		bool found = false;
		while (!found) {
			for (auto t: threads)
				if (t->task_id_begin < 0) {
					t->task_id_end = min((i + cluster_size), n );
					t->task_id_begin = i;
					found = true;
					break;
				}
		}
	}

	// wait till finished
	bool any_running = true;
	while (any_running) {
		any_running = false;
		for (auto t: threads)
			if (t->task_id_begin >= 0)
				any_running = true;
	}

	for (auto t: threads)
		t->f = nullptr;
		//t->join();
}


