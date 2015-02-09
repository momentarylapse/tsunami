#include "../file/file.h"
#include "threads.h"
#include "mutex.h"
#include "work.h"


static int OverwriteThreadNum = -1;

class WorkerThread : public Thread
{
public:
	WorkerThread(int _id, ThreadedWork *_work)
	{
		id = _id;
		work = _work;
	}
	bool Schedule()
	{
		work->mx_list->Lock();
		num = 0;
		if (work->work_given >= work->total_size){
			work->mx_list->Unlock();
			return false;
		}
		first = work->work_given;
		num = min(work->total_size - work->work_given, work->partition_size);
		work->work_given += num;
		work->mx_list->Unlock();
		return true;
	}
	virtual void _cdecl OnRun()
	{
		while(Schedule()){
			for (int i=0;i<num;i++)
				work->DoStep(first + i, id);
		}
	}

	ThreadedWork *work;
	int id;
	int first, num;
};

ThreadedWork::ThreadedWork()
{
	__init__();
}

ThreadedWork::~ThreadedWork()
{
	__delete__();
}

void ThreadedWork::__init__()
{
	thread.__init__();
	mx_list = NULL;
	// use max. number of cores?
	int num_threads = ThreadGetNumCores();
	if (OverwriteThreadNum >= 0)
		num_threads = OverwriteThreadNum;

	for (int i=0;i<num_threads;i++)
		thread.add(new WorkerThread(i, this));
}

void ThreadedWork::__delete__()
{
	foreach(Thread *t, thread)
		delete(t);
}

bool ThreadedWork::Run(int _total_size, int _partition_size)
{
	total_size = _total_size;
	partition_size = _partition_size;
	work_given = 0;

	if (!mx_list)
		mx_list = new Mutex;

	// run threads
	for (int i=0;i<thread.num;i++)
		thread[i]->Run();

	// main program: update gui
	bool all_done = false;
	bool thread_abort = false;
	while((!all_done) && (!thread_abort)){

		thread_abort = !OnStatus();
		all_done = true;
		for (int i=0;i<thread.num;i++)
			all_done &= thread[i]->IsDone();
	}

	if (thread_abort){
		for (int i=0;i<thread.num;i++)
			thread[i]->Kill();
	}else{
		for (int i=0;i<thread.num;i++)
			thread[i]->Join();
	}

	msg_db_l(1);
	return !thread_abort;
}

int ThreadedWork::GetTotal()
{
	return total_size;
}

int ThreadedWork::GetDone()
{
	return work_given;
}

