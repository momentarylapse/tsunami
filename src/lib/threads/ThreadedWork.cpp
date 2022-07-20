#include "../os/msg.h"
#include "Mutex.h"
#include "Thread.h"
#include "ThreadedWork.h"



static int OverwriteThreadNum = -1;

class WorkerThread : public Thread {
public:
	WorkerThread(int _id, ThreadedWork *_work) {
		id = _id;
		work = _work;
		first = 0;
		num = 0;
	}

	bool schedule() {
		work->mx_list->lock();
		num = 0;
		if (work->work_given >= work->total_size) {
			work->mx_list->unlock();
			return false;
		}
		first = work->work_given;
		num = min(work->total_size - work->work_given, work->partition_size);
		work->work_given += num;
		work->mx_list->unlock();
		return true;
	}

	void _cdecl on_run() override {
		while (schedule()) {
			for (int i=0; i<num; i++)
				work->on_step(first + i, id);
		}
	}

	ThreadedWork *work;
	int id;
	int first, num;
};

ThreadedWork::ThreadedWork() {
	partition_size = 1;
	work_given = 0;
	total_size = 0;

	mx_list = nullptr;
	// use max. number of cores?
	int num_threads = Thread::get_num_cores();
	if (OverwriteThreadNum >= 0)
		num_threads = OverwriteThreadNum;

	for (int i=0; i<num_threads; i++)
		thread.add(new WorkerThread(i, this));
}

ThreadedWork::~ThreadedWork() {
	for (Thread *t: thread)
		delete(t);
}

void ThreadedWork::__init__() {
	new(this) ThreadedWork;
}

void ThreadedWork::__delete__() {
	this->ThreadedWork::~ThreadedWork();
}

bool ThreadedWork::run(int _total_size, int _partition_size) {
	total_size = _total_size;
	partition_size = _partition_size;
	work_given = 0;

	if (!mx_list)
		mx_list = new Mutex;

	// run threads
	for (int i=0; i<thread.num; i++)
		thread[i]->run();

	// main program: update gui
	bool all_done = false;
	bool thread_abort = false;
	while (!all_done and !thread_abort) {

		thread_abort = !on_status();
		all_done = true;
		for (int i=0; i<thread.num; i++)
			all_done &= thread[i]->is_done();
	}

	if (thread_abort) {
		for (int i=0; i<thread.num; i++)
			thread[i]->kill();
	} else {
		for (int i=0; i<thread.num; i++)
			thread[i]->join();
	}

	return !thread_abort;
}

int ThreadedWork::get_total() {
	return total_size;
}

int ThreadedWork::get_done() {
	return work_given;
}

