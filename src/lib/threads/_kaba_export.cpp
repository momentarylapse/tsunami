#include <lib/base/base.h>
#include <lib/base/callable.h>
#include "ThreadPool.h"
#include <lib/kapi/KabaExporter.h>


class ThreadPoolWrapper : public ThreadPool {
public:
	void _run(int n, Callable<void(int)>& f, int cluster_size) {
		run(n, [&f] (int i) {
			f(i);
		}, cluster_size);
	}
};

void export_package_threadpool(kaba::IExporter* e) {
	e->package_info("threadpool", "0.1");

	e->declare_class_size("ThreadPool", sizeof(ThreadPool));
	e->link_class_func("ThreadPool.__init__", &kaba::generic_init_ext<ThreadPool, int>);
	e->link_class_func("ThreadPool.__delete__", &kaba::generic_delete<ThreadPool>);
	e->link_class_func("ThreadPool.run", &ThreadPoolWrapper::_run);

}


