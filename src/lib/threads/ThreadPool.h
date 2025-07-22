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

	void run(int n, std::function<void(int)> f, int cluster_size = 1);

	Array<PoolWorkerThread*> threads;
};


#endif //THREADPOOL_H
