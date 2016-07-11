#include "../../file/file.h"
#include "../script.h"
#include "../../config.h"
#include "script_data_common.h"

#ifdef _X_USE_THREADS_
	#include "../../threads/Thread.h"
	#include "../../threads/Mutex.h"
	#include "../../threads/ThreadedWork.h"
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
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,	thread_p(mf(&Thread::__init__)));
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,	thread_p(mf(&Thread::__delete__)));
		class_add_func("run",		TypeVoid,	thread_p(mf(&Thread::run)));
		class_add_func_virtual("onRun",		TypeVoid,	thread_p(mf(&Thread::onRun)));
		class_add_func("isDone",		TypeBool,	thread_p(mf(&Thread::isDone)));
		class_add_func("kill",		TypeVoid,	thread_p(mf(&Thread::kill)));
		class_add_func("join",		TypeVoid,	thread_p(mf(&Thread::join)));
#ifdef _X_USE_THREADS_
		class_set_vtable(Thread);
#endif

	add_class(TypeMutex);
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,	thread_p(mf(&Mutex::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE,		TypeVoid,	thread_p(mf(&Mutex::__delete__)));
		class_add_func("lock",		TypeVoid,	thread_p(mf(&Mutex::lock)));
		class_add_func("unlock",	TypeVoid,	thread_p(mf(&Mutex::unlock)));

	add_class(TypeThreadedWork);
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,	thread_p(mf(&ThreadedWork::__init__)));
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,	thread_p(mf(&ThreadedWork::__delete__)));
		class_add_func("run",		TypeBool,	thread_p(mf(&ThreadedWork::run)));
			func_add_param("total_size", TypeInt);
			func_add_param("partition_size", TypeInt);
		class_add_func_virtual("onStep",		TypeVoid,	thread_p(mf(&ThreadedWork::doStep)));
			func_add_param("index", TypeInt);
			func_add_param("worker_id", TypeInt);
		class_add_func_virtual("onStatus",		TypeBool,	thread_p(mf(&ThreadedWork::onStatus)));
		class_add_func("getTotal",		TypeInt,	thread_p(mf(&ThreadedWork::getTotal)));
		class_add_func("getDone",		TypeInt,	thread_p(mf(&ThreadedWork::getDone)));
#ifdef _X_USE_THREADS_
		class_set_vtable(ThreadedWork);
#endif

	add_func("ThreadGetNumCores",		TypeInt,	thread_p(&Thread::getNumCores));
	add_func("ThreadExit",				TypeVoid,	thread_p(&Thread::exit));
	add_func("ThreadSelf",				TypeThreadP,thread_p(&Thread::getSelf));
}

};
