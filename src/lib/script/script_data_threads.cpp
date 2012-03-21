#include "../file/file.h"
#include "script.h"
#include "../00_config.h"
#include "script_data_common.h"

#ifdef _X_USE_THREADS_
	#include "../threads/threads.h"
#endif


#ifdef _X_USE_THREADS_
	#define thread_p(p)		(void*)p
#else
	#define thread_p(p)		NULL
#endif


void SIAddPackageThread()
{
	msg_db_r("SIAddThread", 3);

	set_cur_package("thread");

	add_func("ThreadGetNumCores",		TypeInt,	thread_p(&ThreadGetNumCores));
	add_func("ThreadCreate",			TypeInt,	thread_p(&ThreadCreate));
		func_add_param("func",				TypePointer);
		func_add_param("param",				TypePointer);
	add_func("ThreadDelete",			TypeVoid,	thread_p(&ThreadDelete));
		func_add_param("thread",			TypeInt);
	add_func("ThreadKill",				TypeVoid,	thread_p(&ThreadKill));
		func_add_param("thread",			TypeInt);
	add_func("ThreadWaitTillDone",		TypeVoid,	thread_p(&ThreadWaitTillDone));
		func_add_param("thread",			TypeInt);
	add_func("ThreadDone",				TypeBool,	thread_p(&ThreadDone));
		func_add_param("thread",			TypeInt);
	add_func("ThreadExit",				TypeVoid,	thread_p(&ThreadExit));
	add_func("ThreadGetId",				TypeInt,	thread_p(&ThreadGetId));
	
	add_func("MutexCreate",				TypeInt,	thread_p(&MutexCreate));
	add_func("MutexLock",				TypeVoid,	thread_p(&MutexLock));
		func_add_param("mutex",				TypeInt);
	add_func("MutexUnlock",				TypeVoid,	thread_p(&MutexUnlock));
		func_add_param("mutex",				TypeInt);
	
	add_func("WorkDo",					TypeBool,	thread_p(&WorkDo));
		func_add_param("func",				TypePointer);
		func_add_param("status_func",		TypePointer);
	add_func("WorkDoScheduled",			TypeBool,	thread_p(&WorkDoScheduled));
		func_add_param("func",				TypePointer);
		func_add_param("status_func",		TypePointer);
		func_add_param("work_size",			TypeInt);
		func_add_param("work_partition",	TypeInt);
	add_func("WorkGetNumThreads",		TypeInt,	thread_p(&WorkGetNumThreads));
	add_func("WorkGetTotal",			TypeInt,	thread_p(&WorkGetTotal));
	add_func("WorkGetDone",				TypeInt,	thread_p(&WorkGetDone));

	msg_db_l(3);
}
