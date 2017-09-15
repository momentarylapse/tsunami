/*
 * Observable.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "Observable.h"
#include <functional>

const string Observable::MESSAGE_ANY = "";
const string Observable::MESSAGE_CHANGE = "Change";
const string Observable::MESSAGE_DELETE = "Delete";
const bool Observable::DEBUG_MESSAGES = true;

static string dummy_string;


static string get_obs_name(VirtualBase *o)
{
	return typeid(*o).name();
}

Observable::Subscription::Subscription()
{
	observer = NULL;
	message = NULL;
}

Observable::Subscription::Subscription(VirtualBase *o, const string *_message, const Observable::Callback &_callback, const Observable::CallbackP &_callback_p)
{
	observer = o;
	message = _message;
	callback = _callback;
	callback_p = _callback_p;
}

Observable::Notification::Notification()
{
	observer = NULL;
	message = NULL;
}

Observable::Notification::Notification(VirtualBase *o, const string *_message, const Observable::Callback &_callback, const Observable::CallbackP &_callback_p)
{
	observer = o;
	message = _message;
	callback = _callback;
	callback_p = _callback_p;
}


Observable::Observable()
{
	observable_notify_level = 0;
	observable_cur_message = NULL;
}

Observable::~Observable()
{
	notify(MESSAGE_DELETE);
}

void Observable::_observable_destruct_()
{
	observable_subscriptions.clear();
	observable_message_queue.clear();
}

void Observable::subscribe2(VirtualBase *o, const Callback &callback, const string &message)
{
	observable_subscriptions.add(Subscription(o, &message, callback, NULL));
}

void Observable::subscribe3(VirtualBase *o, const CallbackP &callback, const string &message)
{
	observable_subscriptions.add(Subscription(o, &message, NULL, callback));
}

void Observable::unsubscribe(VirtualBase *o)
{
	for (int i=observable_subscriptions.num-1; i>=0; i--)
		if (observable_subscriptions[i].observer == o){
			observable_subscriptions.erase(i);
		}
}

void Observable::subscribe_kaba(hui::EventHandler *handler, hui::kaba_member_callback *function)
{
	subscribe2(handler, std::bind(function, handler));
}


void Observable::notifySend()
{
	Array<Notification> notifications;

	// decide whom to send what
	for (const string *m: observable_message_queue){
		//msg_write("send " + observable_name + ": " + queue[i]);
		for (Subscription &r: observable_subscriptions){
			if ((r.message == m) or (r.message == &MESSAGE_ANY))
				notifications.add(Notification(r.observer, m, r.callback, r.callback_p));
		}
	}

	observable_message_queue.clear();

	// send
	for (Notification &n: notifications){
		if (DEBUG_MESSAGES)
			msg_write("send " + get_obs_name(this) + "/" + *n.message + "  >>  " + get_obs_name(n.observer));
		//n.callback();
		observable_cur_message = n.message;
		if (n.callback)
			n.callback();
		else if (n.callback_p)
			n.callback_p(this);
	}
}


void Observable::notifyEnqueue(const string &message)
{
	// already enqueued?
	for (const string *m: observable_message_queue)
		if (&message == m)
			return;

	// add
	observable_message_queue.add(&message);
}

void Observable::notifyBegin()
{
	observable_notify_level ++;
	//msg_write("notify ++");
}

void Observable::notifyEnd()
{
	observable_notify_level --;
	//msg_write("notify --");
	if (observable_notify_level == 0)
		notifySend();
}


void Observable::notify(const string &message)
{
	notifyEnqueue(message);
	if (observable_notify_level == 0)
		notifySend();
}

const string& Observable::cur_message() const
{
	return *observable_cur_message;
}

