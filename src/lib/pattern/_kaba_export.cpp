#include <lib/base/base.h>
#include <lib/base/callable.h>
#include "Observable.h"
#include <lib/kapi/KabaExporter.h>
#include <cstdio>

class Sink;

class InternalNodeData : public obs::internal_node_data {
public:
	obs::sink* create_temp_sink0(VirtualBase* node, Callable<void()>& f) {
		auto s = new obs::sink(node, [&f] { f(); });
		add_temp_sink(s);
		return s;
	}
	obs::xsink<int&>* create_temp_xsink0(VirtualBase* node, Callable<void(void*)>& f) {
		auto s = new obs::xsink<int&>(node, [&f] (int& p) { f(&p); });
		add_temp_sink(s);
		return s;
	}
};

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

// generic source/sink for call-by-reference types
// NOTE void* does not work! (std::function is not compatible between * and &...)
class XPSource : public obs::xsource<int&> {
public:
	void __init__(VirtualBase* node, obs::internal_node_data* ind, const string& name) {
		new(this) obs::base_source(node, name, 0);
		ind->sources.add(this);
	}
};

class XPSink : public obs::xsink<int&> {
public:
	void __init_p__(VirtualBase* node, Callable<void(void*)>& f) {
		//printf("init_p\n");
		new(this) obs::xsink<int&>(node, [&f] (int& p) {
			//printf("XXXX %p\n", &p);
			f(&p);
		});
	}
};

static void xpsubscribe(XPSource* source, XPSink* sink) {
	*source >> *sink;
}




void export_package_obs(kaba::IExporter* e) {
	e->package_info("obs", "0.5");

	e->declare_class_size("InternalNodeData", sizeof(obs::internal_node_data));
	e->link_class_func("InternalNodeData.__init__", &kaba::generic_init<obs::internal_node_data>);
	e->link_class_func("InternalNodeData.__delete__", &kaba::generic_delete<obs::internal_node_data>);
	e->link_class_func("InternalNodeData.cleanup_temp_sinks", &obs::internal_node_data::cleanup_temp_sinks);
	e->link_class_func("InternalNodeData.add_temp_sink", &obs::internal_node_data::add_temp_sink);
	e->link_class_func("InternalNodeData.create_temp_sink0", &InternalNodeData::create_temp_sink0);
	e->link_class_func("InternalNodeData.create_temp_xsink0", &InternalNodeData::create_temp_xsink0);
	e->link_class_func("InternalNodeData.unsubscribe", &obs::internal_node_data::unsubscribe);

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


