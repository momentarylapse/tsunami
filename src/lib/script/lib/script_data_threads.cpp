#include "../../file/file.h"
#include "../script.h"
#include "../../config.h"
#include "script_data_common.h"

#ifdef _X_USE_THREADS_
	#include "../../threads/threads.h"
	#include "../../threads/mutex.h"
	#include "../../threads/work.h"
#endif

namespace Script{


#ifdef _X_USE_THREADS_
	#define thread_p(p)		(void*)p
#else
	typedef int Thread;
	typedef int Mutex;
	typedef int ThreadedWork;
	#define thread_p(p)		NULL
#endif


void SIAddPackageThread()
{
	msg_db_f("SIAddThread", 3);

	add_package("thread", false);

	Type *TypeThread       = add_type  ("Thread",       sizeof(Thread));
	Type *TypeThreadP      = add_type_p("Thread*",      TypeThread);
	Type *TypeMutex        = add_type  ("Mutex",        sizeof(Mutex));
	Type *TypeThreadedWork = add_type  ("ThreadedWork", sizeof(ThreadedWork));

	add_class(TypeThread);
#ifdef _X_USE_THREADS_
		class_set_vtable(Thread);
#endif
		TypeThread->vtable = new VirtualTable[10];
		class_add_func("__init__",		TypeVoid,	thread_p(mf((tmf)&Thread::__init__)));
		class_add_func("__delete__",		TypeVoid,	thread_p(mf((tmf)&Thread::__delete__)));
		class_add_func("Run",		TypeVoid,	thread_p(mf((tmf)&Thread::Run)));
		class_add_func("OnRun",		TypeVoid,	thread_p(mf((tmf)&Thread::OnRun)));
		class_add_func("IsDone",		TypeBool,	thread_p(mf((tmf)&Thread::IsDone)));
		class_add_func("Kill",		TypeVoid,	thread_p(mf((tmf)&Thread::Kill)));
		class_add_func("Join",		TypeVoid,	thread_p(mf((tmf)&Thread::Join)));
		TypeThread->LinkVirtualTable();

	add_class(TypeMutex);
		class_add_func("__init__",		TypeVoid,	thread_p(mf((tmf)&Mutex::__init__)));
		class_add_func("__delete__",		TypeVoid,	thread_p(mf((tmf)&Mutex::__delete__)));
		class_add_func("Lock",		TypeVoid,	thread_p(mf((tmf)&Mutex::Lock)));
		class_add_func("Unlock",	TypeVoid,	thread_p(mf((tmf)&Mutex::Unlock)));

	add_class(TypeThreadedWork);
#ifdef _X_USE_THREADS_
		class_set_vtable(ThreadedWork);
#endif
		TypeThreadedWork->vtable = new VirtualTable[10];
		class_add_func("__init__",		TypeVoid,	thread_p(mf((tmf)&ThreadedWork::__init__)));
		class_add_func("__delete__",		TypeVoid,	thread_p(mf((tmf)&ThreadedWork::__delete__)));
		class_add_func("Run",		TypeBool,	thread_p(mf((tmf)&ThreadedWork::Run)));
			func_add_param("total_size", TypeInt);
			func_add_param("partition_size", TypeInt);
		class_add_func("DoStep",		TypeVoid,	thread_p(mf((tmf)&ThreadedWork::DoStep)));
			func_add_param("index", TypeInt);
			func_add_param("worker_id", TypeInt);
		class_add_func("OnStatus",		TypeBool,	thread_p(mf((tmf)&ThreadedWork::OnStatus)));
		class_add_func("GetTotal",		TypeInt,	thread_p(mf((tmf)&ThreadedWork::GetTotal)));
		class_add_func("GetDone",		TypeInt,	thread_p(mf((tmf)&ThreadedWork::GetDone)));
		TypeThreadedWork->LinkVirtualTable();

	add_func("ThreadGetNumCores",		TypeInt,	thread_p(&ThreadGetNumCores));
	add_func("ThreadExit",				TypeVoid,	thread_p(&ThreadExit));
	add_func("ThreadSelf",				TypeThreadP,thread_p(&ThreadSelf));
}

};
