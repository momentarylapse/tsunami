/*
 * Observable.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "Observable.h"
#include <functional>
//#include <typeinfo>
#include "../kaba/kaba.h"
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
	if (kaba::default_context) {
		if (auto *c = kaba::default_context->get_dynamic_type(o))
			return "<kaba:" + c->name + ">" + pp;
	}
	return printify(typeid(*o).name()) + pp;
}

ObservableData::Subscription::Subscription() {
	observer = nullptr;
	message = nullptr;
}

ObservableData::Subscription::Subscription(VirtualBase *o, const string *_message, const ObservableData::Callback &_callback) {
	observer = o;
	message = _message;
	callback = _callback;
}

ObservableData::Notification::Notification() {
	observer = nullptr;
	message = nullptr;
}

ObservableData::Notification::Notification(VirtualBase *o, const string *_message, const ObservableData::Callback &_callback) {
	observer = o;
	message = _message;
	callback = _callback;
}


ObservableData::ObservableData() {
	me = nullptr; // it is only useful, when set by subscribe anyways...
}

ObservableData::~ObservableData() {
	if constexpr (MESSAGE_DEBUG_LEVEL >= 2)
		msg_write("~ObservableData");
	if (me) {
		if constexpr (MESSAGE_DEBUG_LEVEL >= 2)
			msg_write("notify... " + get_obs_name(me));
		//notify(me->MESSAGE_DELETE);
		notify(LegacyObservable<VirtualBase>::MESSAGE_DELETE);
	}
}

// in case an object is "out of service" but still needs to be kept in memory
void ObservableData::fake_death() {
	notify(LegacyObservable<VirtualBase>::MESSAGE_DELETE);
	subscriptions.clear();
}


void ObservableData::subscribe(VirtualBase *_me, VirtualBase *observer, const ObservableData::Callback &callback, const string &message) {
	subscriptions.add(ObservableData::Subscription(observer, &message, callback));
	me = _me;
	if constexpr (MESSAGE_DEBUG_LEVEL >= 2)
		msg_write(format("subscribe:  %s  <<  (%s)  <<  %s", get_obs_name(me), message, get_obs_name(observer)));
}

void ObservableData::unsubscribe(VirtualBase *observer) {
	if constexpr (MESSAGE_DEBUG_LEVEL >= 2)
		msg_write(format("unsubscribe:  %s  <<  %s", get_obs_name(me), get_obs_name(observer)));
	for (int i=subscriptions.num-1; i>=0; i--)
		if (subscriptions[i].observer == observer) {
			subscriptions.erase(i);
		}
}

static bool om_match(const string &m, const ObservableData::Subscription &r) {
	if (*r.message == m)
		return true;
	if ((*r.message == LegacyObservable<VirtualBase>::MESSAGE_ANY) and (m != LegacyObservable<VirtualBase>::MESSAGE_DELETE))
		return true;
	return false;
}


void ObservableData::notify(const string &message) {
	if (!me)
		return;

	Array<Notification> notifications;

	// decide whom to send what
	//msg_write("send " + observable_name + ": " + queue[i]);
	for (auto &r: subscriptions) {
		if (om_match(message, r))
			notifications.add(Notification(r.observer, &message, r.callback));

		// why don't we just send in this loop?
		// I guess it has something to do with recursion... but what...???
		// still needed without grouping/etc
	}

	// send
	for (auto &n: notifications) {
		if constexpr (MESSAGE_DEBUG_LEVEL >= 1) {
			msg_write(format("send %s  ---%s--->> %s", get_obs_name(me), message, get_obs_name(n.observer)));
			msg_right();
		}

		//cur_message = n.message;
		if (n.callback)
			n.callback();

		if constexpr (MESSAGE_DEBUG_LEVEL >= 1) {
			msg_left();
		}
	}
}


namespace obs {
Source::Source(VirtualBase* _node, const string& _name, ObservableData* obs_data) {
	node = _node;
	name = _name;
	legacy_observable_data = obs_data;
	if constexpr (NODE_DEBUG_LEVEL >= 2)
		msg_write(format("add  %s . %s", get_obs_name(node), name));
}
Source::~Source() {
	if constexpr (NODE_DEBUG_LEVEL >= 2)
		msg_write(format("del  %s . %s", get_obs_name(node), name));
	auto _sinks = sinks;
	for (auto s: _sinks)
		unsubscribe(*s);
}
void Source::notify() const {
	for (auto s: sinks) {
		if constexpr (NODE_DEBUG_LEVEL >= 2)
			msg_write(format("send  %s  ---%s--->>  %s", get_obs_name(node), name, get_obs_name(s->node)));
		s->callback();
	}
	if (legacy_observable_data)
		legacy_observable_data->notify(name);
}
void Source::subscribe(Sink& sink) {
	if constexpr (NODE_DEBUG_LEVEL >= 2)
		msg_write(format("subs  %s . %s  >  %s", get_obs_name(node), name, get_obs_name(sink.node)));
	sinks.add(&sink);
	sink.sources.add(this);
}
void Source::unsubscribe(Sink& sink) {
	_remove(&sink);
	sink._remove(this);
}
void Source::unsubscribe(VirtualBase* node) {
	auto _sinks = sinks;
	for (auto s: _sinks)
		if (s->node == node)
			unsubscribe(*s);
}
void Source::operator >>(Sink& sink) {
	subscribe(sink);
}
void Source::_remove(Sink* sink) {
	base::remove(sinks, sink);
}


Sink::Sink(VirtualBase* n, Callback c) {
	node = n;
	callback = c;
}
Sink::~Sink() {
	if constexpr (NODE_DEBUG_LEVEL >= 2)
		msg_write(format("del  %s  (sink)", get_obs_name(node)));
	auto _sources = sources;
	for (auto s: _sources)
		s->unsubscribe(*this);
}
void Sink::_remove(Source* source) {
	base::remove(sources, source);
}
}

