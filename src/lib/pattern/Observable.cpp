/*
 * Observable.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "Observable.h"
#include <functional>
//#include <typeinfo>
#if __has_include("../kaba/kaba.h")
#include "../kaba/kaba.h"
#define HAS_KABA
#endif
#include "../os/msg.h"
#include "../base/algo.h"


// notify()
//   should be called from the main/gui thread!


static const int MESSAGE_DEBUG_LEVEL = 0;
static const int NODE_DEBUG_LEVEL = 0;

bool split_num_string(const string &n, string &out, int &after) {
	out = n;
	if (n[0] < '0' or n[0] > '9')
		return false;
	if (n[1] >= '0' and n[1] <= '9') {
		int num = n.head(2)._int();
		out = n.sub(2, 2 + num);
		after = num + 2;
		return true;
	}
	int num = n.head(1)._int();
	out = n.sub(1, 1+num);
	after = num + 1;
	return true;
}

string printify(const string &n) {
	string out;
	int after;
	if (!split_num_string(n,  out, after))
		return n;
	if (after < n.num) {
		if (n[after] == 'I') {
			int after2;
			string out2;
			if (!split_num_string(n.sub(after+1),  out2, after2))
				return n + format("----a %d", after);
			return out + "<" + out2 + ">";
		} else {
			return n + format("----b %d", after);
		}
	}
	return out;
}

string get_obs_name(VirtualBase *o) {
	if (!o)
		return "<null>";
	string pp;
	if constexpr (MESSAGE_DEBUG_LEVEL >= 4)
		pp = "(" + p2s(o) + ")";
#ifdef HAS_KABA
	if (kaba::default_context) {
		if (auto *c = kaba::default_context->get_dynamic_type(o))
			return "<kaba:" + c->name + ">" + pp;
	}
#endif
	return printify(typeid(*o).name()) + pp;
}


/**************************************
 * source
 **************************************/

namespace obs {
base_source::base_source(VirtualBase* _node, const string& _name, int) {
	node = _node;
	name = _name;
	if constexpr (NODE_DEBUG_LEVEL >= 2)
		msg_write(format("add  %s . %s", get_obs_name(node), name));
}

base_source::~base_source() {
	if constexpr (NODE_DEBUG_LEVEL >= 2)
		msg_write(format("del  %s . %s", get_obs_name(node), name));
	auto _sinks = connected_sinks;
	for (auto s: _sinks)
		unsubscribe(*s);
}

#if 0
void source::notify() const {
	for (auto s: connected_sinks) {
		if constexpr (NODE_DEBUG_LEVEL >= 2)
			msg_write(format("send  %s  ---%s--->>  %s", get_obs_name(node), name, get_obs_name(s->node)));
		s->callback();
	}
}
#endif


void base_source::_notify() const {
	if constexpr (NODE_DEBUG_LEVEL >= 2) {
		for (const base_sink* s: connected_sinks)
			msg_write(format("send  %s  ---%s--->>  %s", get_obs_name(node), name, get_obs_name(s->node)));
	}
}

void base_source::_subscribe(base_sink& sink) {
	if constexpr (NODE_DEBUG_LEVEL >= 2)
		msg_write(format("subs  %s . %s  >  %s", get_obs_name(node), name, get_obs_name(sink.node)));
	connected_sinks.add(&sink);
	sink.connected_sources.add(this);
}

void base_source::unsubscribe(base_sink& s) {
	remove_sink(&s);
	s.remove_source(this);
}

void base_source::unsubscribe(VirtualBase* node) {
	auto _connected_sinks = connected_sinks;
	for (auto s: _connected_sinks)
		if (s->node == node)
			unsubscribe(*s);
}

#if 0
void source::operator >>(sink& sink) {
	subscribe(sink);
}
#endif

void base_source::remove_sink(base_sink* s) {
	base::remove(connected_sinks, s);
}


/**************************************
 * sink
 **************************************/

#if 0
void sink::init(VirtualBase* n, Callback c) {
	node = n;
	callback = c;
}
#endif

base_sink::~base_sink() {
	if constexpr (NODE_DEBUG_LEVEL >= 2)
		msg_write(format("del  %s  (sink)", get_obs_name(node)));
	auto _connected_sources = connected_sources;
	for (auto s: _connected_sources)
		s->unsubscribe(*this);
}

void base_sink::remove_source(base_source* s) {
	base::remove(connected_sources, s);
}


/**************************************
 * internal_node_data
 **************************************/

internal_node_data::~internal_node_data() {
	for (auto s: temp_sinks)
		delete s;
}

#if 0
sink& internal_node_data::create_sink(VirtualBase* node, Callback callback) {
	cleanup_temp_sinks();
	temp_sinks.add(new sink(node, callback));
	return *temp_sinks.back();
}
#endif

void internal_node_data::cleanup_temp_sinks() {
	for (int i=0; i<temp_sinks.num; i++)
		if (temp_sinks[i]->connected_sources.num == 0) {
			delete temp_sinks[i];
			temp_sinks.erase(i);
			i --;
		}
}

void internal_node_data::unsubscribe(VirtualBase *observer) {
	for (auto s: sources)
		s->unsubscribe(observer);
}

}

