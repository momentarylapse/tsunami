/*----------------------------------------------------------------------------*\
| Threads (work scheduler)                                                     |
|                                                                              |
| last update: 2011.02.19 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#pragma once

#include "Mutex.h"
#include "Thread.h"

#define MAX_THREADS			32

class ThreadedWork : public VirtualBase {
public:
	ThreadedWork();
	virtual ~ThreadedWork();
	virtual void _cdecl on_step(int index, int worker_id) {}
	virtual bool _cdecl on_status() { return true; }
	bool _cdecl run(int total_size, int partition_size);

	int total_size, partition_size;
	Array<Thread*> thread;

	int work_given;
	Mutex *mx_list;

	int _cdecl get_total();
	int _cdecl get_done();

	void _cdecl __init__();
	virtual void _cdecl __delete__();
};


