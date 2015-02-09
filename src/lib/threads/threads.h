/*----------------------------------------------------------------------------*\
| Threads                                                                      |
|                                                                              |
| last update: 2011.02.19 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#if !defined(THREADS_H_INCLUDED)
#define THREADS_H_INCLUDED

#include "../base/base.h"

// auxiliary
int ThreadGetNumCores();

//typedef void thread_func_t(void*);
//typedef bool thread_status_func_t();

struct ThreadInternal;

class Thread : public VirtualBase
{
public:
	Thread();
	virtual ~Thread();
	void _cdecl Run();
	bool _cdecl IsDone();
	void _cdecl Kill();
	void _cdecl Join();

	virtual void _cdecl OnRun(){}// = 0;

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	bool running;
	ThreadInternal *internal;
};

void _cdecl ThreadExit();
Thread *_cdecl ThreadSelf();


#endif

