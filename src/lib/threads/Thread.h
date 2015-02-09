/*----------------------------------------------------------------------------*\
| Threads                                                                      |
|                                                                              |
| last update: 2011.02.19 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#if !defined(THREAD_H_INCLUDED)
#define THREAD_H_INCLUDED

#include "../base/base.h"


struct ThreadInternal;

class Thread : public VirtualBase
{
public:
	Thread();
	virtual ~Thread();
	void _cdecl run();
	bool _cdecl isDone();
	void _cdecl kill();
	void _cdecl join();

	virtual void _cdecl onRun(){}// = 0;

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	bool running;
	ThreadInternal *internal;


	// auxiliary
	static int _cdecl getNumCores();

	static void _cdecl exit();
	static Thread *_cdecl getSelf();
};


#endif

