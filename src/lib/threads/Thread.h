/*----------------------------------------------------------------------------*\
| Threads                                                                      |
|                                                                              |
| last update: 2011.02.19 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#if !defined(THREAD_H_INCLUDED)
#define THREAD_H_INCLUDED

#include "../base/base.h"
#include <atomic>
#include <mutex>


struct ThreadInternal;

class Thread : public VirtualBase
{
public:
	Thread();
	virtual ~Thread();
	void _cdecl run();
	bool _cdecl is_done();
	void _cdecl kill();
	void _cdecl join();

	virtual void _cdecl on_run(){}// = 0;
	virtual void _cdecl on_cancel(){}

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	std::atomic<bool> done;
	std::atomic<bool> running;
	ThreadInternal *internal;
	std::mutex control_mutex;


	// auxiliary
	static int _cdecl get_num_cores();

	static void _cdecl exit();
	static Thread *_cdecl get_self();
	static void cancelation_point();
};


#endif

