/*----------------------------------------------------------------------------*\
| Threads (work scheduler)                                                     |
|                                                                              |
| last update: 2011.02.19 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#if !defined(WORK_H_INCLUDED)
#define WORK_H_INCLUDED

#include "threads.h"
#include "mutex.h"

#define MAX_THREADS			32

class ThreadedWork : public VirtualBase
{
public:
	ThreadedWork();
	virtual ~ThreadedWork();
	virtual void _cdecl DoStep(int index, int worker_id){}
	virtual bool _cdecl OnStatus(){	return true;	}
	bool _cdecl Run(int total_size, int partition_size);

	int total_size, partition_size;
	Array<Thread*> thread;

	int work_given;
	Mutex *mx_list;

	int _cdecl GetTotal();
	int _cdecl GetDone();

	void _cdecl __init__();
	virtual void _cdecl __delete__();
};


#endif

