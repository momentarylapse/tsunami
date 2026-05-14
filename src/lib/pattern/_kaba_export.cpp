#include <lib/base/base.h>
#include <lib/base/callable.h>
#include "Observable.h"
#include <lib/kapi/KabaExporter.h>
#include <cstdio>


class Source : public obs::source {
public:
	void __init__(VirtualBase* node, obs::internal_node_data* ind, const string& name) {
		new(this) obs::base_source(node, name, 0);
		ind->sources.add(this);
	}
};

class Sink : public obs::sink {
public:
	void __init__(VirtualBase* node, Callable<void()>& f) {
		new(this) obs::sink(node, [&f] { f(); });
	}
};

static void subscribe(Source* source, Sink* sink) {
	*source >> *sink;
}


class XPSource : public obs::xsource<void*> {
public:
	void __init__(VirtualBase* node, obs::internal_node_data* ind, const string& name) {
		new(this) obs::base_source(node, name, 0);
		ind->sources.add(this);
	}
};

class XPSink : public obs::xsink<void*> {
public:
	void __init_p__(VirtualBase* node, Callable<void(void*)>& f) {
		//printf("init_p\n");
		//f(nullptr);
		new(this) obs::xsink<void*>(node, [&f] (void* p) {
			//printf("XXXX %p\n", p);
			f(p);
		});
	}
};

static void xpsubscribe(XPSource* source, XPSink* sink) {
	*source >> *sink;
}




void export_package_obs(kaba::IExporter* e) {
	e->package_info("obs", "0.1");

	e->declare_class_size("InternalNodeData", sizeof(obs::internal_node_data));
	e->link_class_func("InternalNodeData.__init__", &kaba::generic_init<obs::internal_node_data>);
	e->link_class_func("InternalNodeData.__delete__", &kaba::generic_delete<obs::internal_node_data>);

	e->declare_class_size("Source", sizeof(Source));
	e->link_class_func("Source.__init__", &Source::__init__);
	e->link_class_func("Source.notify", &Source::notify);

	e->declare_class_size("Sink", sizeof(Sink));
	e->link_class_func("Sink.__init2__", &Sink::__init__);
	
	e->link_func("subscribe", &subscribe);

	e->declare_class_size("XPSource", sizeof(XPSource));
	e->link_class_func("XPSource.__init__", &XPSource::__init__);
	e->link_class_func("XPSource._notify", &XPSource::notify);

	e->declare_class_size("XPSink", sizeof(XPSink));
	e->link_class_func("XPSink.__init0__", &XPSink::__init_p__);
	
	e->link_func("xpsubscribe", &xpsubscribe);
}


