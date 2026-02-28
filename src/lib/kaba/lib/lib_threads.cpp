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
	add_internal_package(c, "thread", "1");

	const Class *TypeThread    = add_type("Thread", sizeof(Thread));
	const Class *TypeThreadP   = add_type_p_raw(TypeThread);
	const Class *TypeMutex     = add_type("Mutex", sizeof(Mutex));
	const Class *TypeScheduler = add_type("Scheduler", sizeof(ThreadedWork));

	add_class(TypeThread);
		class_add_func(Identifier::func::Init, common_types._void, thread_p(&Thread::__init__), Flags::Mutable);
		class_add_func_virtual(Identifier::func::Delete, common_types._void, thread_p(&Thread::__delete__), Flags::Mutable);
		class_add_func("run", common_types._void, thread_p(&Thread::run), Flags::Mutable);
		class_add_func_virtual("on_run", common_types._void, thread_p(&Thread::on_run));
		class_add_func("is_done", common_types._bool, thread_p(&Thread::is_done));
		class_add_func("kill", common_types._void, thread_p(&Thread::kill), Flags::Mutable);
		class_add_func("join", common_types._void, thread_p(&Thread::join), Flags::Mutable);
		class_add_func("self", TypeThreadP,thread_p(&Thread::get_self), Flags::Static);
#ifdef KABA_EXPORT_THREADS
		class_set_vtable(Thread);
#endif

	add_class(TypeMutex);
		class_add_func(Identifier::func::Init, common_types._void, thread_p(&Mutex::__init__), Flags::Mutable);
		class_add_func(Identifier::func::Delete, common_types._void, thread_p(&Mutex::__delete__), Flags::Mutable);
		class_add_func("lock", common_types._void, thread_p(&Mutex::lock), Flags::Mutable);
		class_add_func("unlock", common_types._void, thread_p(&Mutex::unlock), Flags::Mutable);

	add_class(TypeScheduler);
		class_add_func(Identifier::func::Init, common_types._void, thread_p(&ThreadedWork::__init__), Flags::Mutable);
		class_add_func_virtual(Identifier::func::Delete, common_types._void, thread_p(&ThreadedWork::__delete__), Flags::Mutable);
		class_add_func("run", common_types._bool, thread_p(&ThreadedWork::run), Flags::Mutable);
			func_add_param("total_size", common_types.i32);
			func_add_param("partition_size", common_types.i32);
		class_add_func_virtual("on_step", common_types._void, thread_p(&ThreadedWork::on_step));
			func_add_param("index", common_types.i32);
			func_add_param("worker_id", common_types.i32);
		class_add_func_virtual("on_status", common_types._bool, thread_p(&ThreadedWork::on_status));
		class_add_func("get_total", common_types.i32, thread_p(&ThreadedWork::get_total));
		class_add_func("get_done", common_types.i32, thread_p(&ThreadedWork::get_done));
#ifdef KABA_EXPORT_THREADS
		class_set_vtable(ThreadedWork);
#endif


	add_func("get_num_cores", common_types.i32, thread_p(&Thread::get_num_cores), Flags::Static);
	add_func("exit", common_types._void, thread_p(&Thread::exit), Flags::Static);
}

};
