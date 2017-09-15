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


/*const string MESSAGE_CHANGE = "Change";
const string MESSAGE_DELETE = "Delete";
const string MESSAGE_ANY = "";*/
static const bool DEBUG_MESSAGES = true;

static string dummy_string;

namespace Kaba{
extern Array<Kaba::Script*> PublicScript;
};

static string get_obs_name(VirtualBase *o)
{
	for (auto s: Kaba::PublicScript)
		for (auto c: s->syntax->classes){
			void *vtable = *(void**)o;
			if (vtable == c->_vtable_location_compiler_)
				return "<kaba:" + c->name + ">";
		}
	return typeid(*o).name();

	//return p2s(o); // sigh... m(-_-)m

	/*try{
		o = dynamic_cast<VirtualBase*>(o);
	}catch(...){
		return "???";
	}

	msg_write("a");
	const std::type_info &a = typeid(*o);
	msg_write("b");
	return a.name();*/
}

ObservableData::Subscription::Subscription()
{
	observer = NULL;
	message = NULL;
}

ObservableData::Subscription::Subscription(VirtualBase *o, const string *_message, const ObservableData::Callback &_callback, const ObservableData::CallbackP &_callback_p)
{
	observer = o;
	message = _message;
	callback = _callback;
	callback_p = _callback_p;
}

ObservableData::Notification::Notification()
{
	observer = NULL;
	message = NULL;
}

ObservableData::Notification::Notification(VirtualBase *o, const string *_message, const ObservableData::Callback &_callback, const ObservableData::CallbackP &_callback_p)
{
	observer = o;
	message = _message;
	callback = _callback;
	callback_p = _callback_p;
}


ObservableData::ObservableData()
{
	me = NULL; // it is only useful, when set by subscribe anyways...
	notify_level = 0;
	cur_message = NULL;
}

ObservableData::~ObservableData()
{
	msg_write("~ObservableData");
	if (me){
		//notify(me->MESSAGE_DELETE);
		msg_write("notify... " + get_obs_name(me));
		notify(Observable<VirtualBase>::MESSAGE_DELETE);
	}
}

void ObservableData::unsubscribe(VirtualBase *o)
{
	for (int i=subscriptions.num-1; i>=0; i--)
		if (subscriptions[i].observer == o){
			subscriptions.erase(i);
		}
}

void ObservableData::notifySend()
{
	if (!me){
		message_queue.clear();
		return;
	}

	Array<Notification> notifications;

	// decide whom to send what
	for (const string *m: message_queue){
		//msg_write("send " + observable_name + ": " + queue[i]);
		for (Subscription &r: subscriptions){
			if ((r.message == m) or (*r.message == Observable<VirtualBase>::MESSAGE_ANY))
				notifications.add(Notification(r.observer, m, r.callback, r.callback_p));
		}
	}

	message_queue.clear();

	// send
	for (Notification &n: notifications){
		if (DEBUG_MESSAGES)
			msg_write("send " + get_obs_name(me) + "  >>  " + *n.message + "  >>  " + get_obs_name(n.observer));
		//n.callback();
		cur_message = n.message;
		if (n.callback)
			n.callback();
		else if (n.callback_p)
			n.callback_p(me);
	}
}


void ObservableData::notifyEnqueue(const string &message)
{
	// already enqueued?
	for (const string *m: message_queue)
		if (&message == m)
			return;

	// add
	message_queue.add(&message);
}

void ObservableData::notifyBegin()
{
	notify_level ++;
	//msg_write("notify ++");
}

void ObservableData::notifyEnd()
{
	notify_level --;
	//msg_write("notify --");
	if (notify_level == 0)
		notifySend();
}


void ObservableData::notify(const string &message)
{
	notifyEnqueue(message);
	if (notify_level == 0)
		notifySend();
}

