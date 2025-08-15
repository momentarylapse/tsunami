//
// Created by michi on 7/21/25.
//

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "../base/base.h"
#include "Thread.h"
#include <functional>

class PoolWorkerThread;

class ThreadPool {
public:
	explicit ThreadPool(int num_threads = -1);
	~ThreadPool();

	template<typename F>
	void _run_single_threaded(int n, F f) {
		for (int i=0; i < n; i++)
			f(i);
	}

	template<typename F>
	void run(int n, F f, int cluster_size = 1) {
		if (n < cluster_size * 2) {
			_run_single_threaded(n, f);
			return;
		}
		_begin();
		for (int i0 = 0; i0 < n; i0+=cluster_size) {
			_emit_cluster([i0, n, cluster_size, f] {
				int i1 = min(n, i0 + cluster_size);
				for (int i=i0; i<i1; i++) {
					f(i);
				}
			});
		}
		_end();
	}

	void _begin();
	void _end();
	void _emit_cluster(std::function<void()> f);

	Array<PoolWorkerThread*> threads;
};


#endif //THREADPOOL_H
