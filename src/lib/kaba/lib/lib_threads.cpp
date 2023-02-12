#include "../kaba.h"
#include "lib.h"

#if __has_include("../../threads/Thread.h")
	#include "../../threads/Thread.h"
	#include "../../threads/Mutex.h"
	#include "../../threads/ThreadedWork.h"
	#define KABA_EXPORT_THREADS
#endif

namespace kaba {


#ifdef KABA_EXPORT_THREADS
	#define thread_p(p)		p
#else
	typedef int Thread;
	typedef int Mutex;
	typedef int ThreadedWork;
	#define thread_p(p)		nullptr
#endif


void SIAddPackageThread(Context *c) {
	add_package(c, "thread");

	const Class *TypeThread       = add_type  ("Thread", sizeof(Thread));
	const Class *TypeThreadP      = add_type_p(TypeThread);
	const Class *TypeMutex        = add_type  ("Mutex", sizeof(Mutex));
	const Class *TypeThreadedWork = add_type  ("ThreadedWork", sizeof(ThreadedWork));

	add_class(TypeThread);
		class_add_func(Identifier::Func::INIT, TypeVoid, thread_p(&Thread::__init__));
		class_add_func_virtual(Identifier::Func::DELETE, TypeVoid, thread_p(&Thread::__delete__));
		class_add_func("run", TypeVoid, thread_p(&Thread::run));
		class_add_func_virtual("on_run", TypeVoid, thread_p(&Thread::on_run));
		class_add_func("is_done", TypeBool, thread_p(&Thread::is_done));
		class_add_func("kill", TypeVoid, thread_p(&Thread::kill));
		class_add_func("join", TypeVoid, thread_p(&Thread::join));
		class_add_func("get_num_cores", TypeInt, thread_p(&Thread::get_num_cores), Flags::STATIC);
		class_add_func("exit", TypeVoid, thread_p(&Thread::exit), Flags::STATIC);
		class_add_func("self", TypeThreadP,thread_p(&Thread::get_self), Flags::STATIC);
#ifdef KABA_EXPORT_THREADS
		class_set_vtable(Thread);
#endif

	add_class(TypeMutex);
		class_add_func(Identifier::Func::INIT, TypeVoid, thread_p(&Mutex::__init__));
		class_add_func(Identifier::Func::DELETE, TypeVoid, thread_p(&Mutex::__delete__));
		class_add_func("lock", TypeVoid, thread_p(&Mutex::lock));
		class_add_func("unlock", TypeVoid, thread_p(&Mutex::unlock));

	add_class(TypeThreadedWork);
		class_add_func(Identifier::Func::INIT, TypeVoid, thread_p(&ThreadedWork::__init__));
		class_add_func_virtual(Identifier::Func::DELETE, TypeVoid, thread_p(&ThreadedWork::__delete__));
		class_add_func("run", TypeBool, thread_p(&ThreadedWork::run));
			func_add_param("total_size", TypeInt);
			func_add_param("partition_size", TypeInt);
		class_add_func_virtual("on_step", TypeVoid, thread_p(&ThreadedWork::on_step));
			func_add_param("index", TypeInt);
			func_add_param("worker_id", TypeInt);
		class_add_func_virtual("on_status", TypeBool, thread_p(&ThreadedWork::on_status));
		class_add_func("get_total", TypeInt, thread_p(&ThreadedWork::get_total));
		class_add_func("get_done", TypeInt, thread_p(&ThreadedWork::get_done));
#ifdef KABA_EXPORT_THREADS
		class_set_vtable(ThreadedWork);
#endif
}

};
