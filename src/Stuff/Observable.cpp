/*
 * Observable.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "Observable.h"
#include <functional>
//#include <typeinfo>
#include "../lib/kaba/kaba.h"
#include "../lib/file/msg.h"


// notify()
//   should be called from the main/gui thread!


static const int MESSAGE_DEBUG_LEVEL = 0;

static string dummy_string;

bool split_num_string(const string &n, string &out, int &after) {
	out = n;
	if (n[0] < '0' or n[0] > '9')
		return false;
	if (n[1] >= '0' and n[1] <= '9') {
		int num = n.head(2)._int();
		out = n.substr(2, num);
		after = num + 2;
		return true;
	}
	int num = n.head(1)._int();
	out = n.substr(1, num);
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
			if (!split_num_string(n.substr(after+1, -1),  out2, after2))
				return n + format("----a %d", after);
			return out + "<" + out2 + ">";
		} else {
			return n + format("----b %d", after);
		}
	}
	return out;
}

static string get_obs_name(VirtualBase *o) {
	if (!o)
		return "<null>";
	string pp;
	if (MESSAGE_DEBUG_LEVEL >= 4)
		pp = "(" + p2s(o) + ")";
	auto *c = kaba::get_dynamic_type(o);
	if (c)
		return "<kaba:" + c->name + ">" + pp;
	return printify(typeid(*o).name()) + pp;
}

ObservableData::Subscription::Subscription() {
	observer = nullptr;
	message = nullptr;
}

ObservableData::Subscription::Subscription(VirtualBase *o, const string *_message, const ObservableData::Callback &_callback, const ObservableData::CallbackP &_callback_p) {
	observer = o;
	message = _message;
	callback = _callback;
	callback_p = _callback_p;
}

ObservableData::Notification::Notification() {
	observer = nullptr;
	message = nullptr;
}

ObservableData::Notification::Notification(VirtualBase *o, const string *_message, const ObservableData::Callback &_callback, const ObservableData::CallbackP &_callback_p) {
	observer = o;
	message = _message;
	callback = _callback;
	callback_p = _callback_p;
}


ObservableData::ObservableData() {
	me = nullptr; // it is only useful, when set by subscribe anyways...
	notify_level = 0;
	cur_message = nullptr;
}

ObservableData::~ObservableData() {
	if (MESSAGE_DEBUG_LEVEL >= 2)
		msg_write("~ObservableData");
	if (me) {
		if (MESSAGE_DEBUG_LEVEL >= 2)
			msg_write("notify... " + get_obs_name(me));
		//notify(me->MESSAGE_DELETE);
		notify(Observable<VirtualBase>::MESSAGE_DELETE);
	}
}

// in case an object is "out of service" but still needs to be kept in memory
void ObservableData::fake_death() {
	notify(Observable<VirtualBase>::MESSAGE_DELETE);
	subscriptions.clear();
}


void ObservableData::subscribe(VirtualBase *_me, VirtualBase *observer, const ObservableData::Callback &callback, const ObservableData::CallbackP &callback_p, const string &message) {
	subscriptions.add(ObservableData::Subscription(observer, &message, callback, callback_p));
	me = _me;
	if (MESSAGE_DEBUG_LEVEL >= 2)
		msg_write(format("subscribe:  %s  <<  (%s)  <<  %s", get_obs_name(me), message, get_obs_name(observer)));
}

void ObservableData::unsubscribe(VirtualBase *observer) {
	if (MESSAGE_DEBUG_LEVEL >= 2)
		msg_write(format("unsubscribe:  %s  <<  %s", get_obs_name(me), get_obs_name(observer)));
	for (int i=subscriptions.num-1; i>=0; i--)
		if (subscriptions[i].observer == observer) {
			subscriptions.erase(i);
		}
}

static bool om_match(const string &m, const ObservableData::Subscription &r) {
	if (*r.message == m)
		return true;
	if ((*r.message == Observable<VirtualBase>::MESSAGE_ANY) and (m != Observable<VirtualBase>::MESSAGE_DELETE))
		return true;
	return false;
}

void ObservableData::notify_send() {
	if (!me) {
		message_queue.clear();
		return;
	}

	Array<Notification> notifications;

	// decide whom to send what
	for (const string *m: message_queue) {
		//msg_write("send " + observable_name + ": " + queue[i]);
		for (auto &r: subscriptions) {
			if (om_match(*m, r))
				notifications.add(Notification(r.observer, m, r.callback, r.callback_p));
		}
	}

	message_queue.clear();

	// send
	for (auto &n: notifications) {
		if (MESSAGE_DEBUG_LEVEL >= 1) {
			msg_write(format("send %s  ---%s--->> %s", get_obs_name(me), *n.message, get_obs_name(n.observer)));
			msg_right();
		}

		cur_message = n.message;
		if (n.callback)
			n.callback();
		else if (n.callback_p)
			n.callback_p(me);

		if (MESSAGE_DEBUG_LEVEL >= 1) {
			msg_left();
		}
	}
}


void ObservableData::notify_enqueue(const string &message) {
	// already enqueued?
	for (const string *m: message_queue)
		if (&message == m)
			return;

	// add
	message_queue.add(&message);
}

void ObservableData::notify_begin() {
	notify_level ++;
}

void ObservableData::notify_end() {
	notify_level --;
	if (notify_level == 0)
		notify_send();
}


void ObservableData::notify(const string &message) {
	notify_enqueue(message);
	if (notify_level == 0)
		notify_send();
}

