/*
 * Observable.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "Observable.h"
#include "Observer.h"

const string Observable::MESSAGE_ALL = "";
const string Observable::MESSAGE_CHANGE = "Change";
const string Observable::MESSAGE_DELETE = "Delete";
const bool Observable::DEBUG_MESSAGES = false;


struct ObserverRequest
{
	ObserverRequest(){}
	ObserverRequest(Observer *o, const string &_message)
	{
		observer = o;
		message = &_message;
	}
	Observer* observer;
	const string *message;
};

typedef ObserverRequest Notification;


Observable::Observable(const string &name)
{
	notify_level = 0;
	observable_name = name;
}

Observable::~Observable()
{
	notify(MESSAGE_DELETE);
}

void Observable::addObserver(Observer *o, const string &message)
{
	requests.add(ObserverRequest(o, message));
}

void Observable::removeObserver(Observer *o)
{
	for (int i=requests.num-1; i>=0; i--)
		if (requests[i].observer == o){
			requests.erase(i);
		}
}

void Observable::addWrappedObserver(void* handler, void* func)
{
	Observer *o = new ObserverWrapper(handler, func);
	addObserver(o, MESSAGE_ALL);
}

void Observable::removeWrappedObserver(void* handler)
{
	foreachi(ObserverRequest &r, requests, i)
		if (dynamic_cast<ObserverWrapper*>(r.observer)){
			if (dynamic_cast<ObserverWrapper*>(r.observer)->handler == handler){
				delete(r.observer);
				requests.erase(i);
				break;
			}
		}
}



string Observable::getName()
{	return observable_name;	}

void Observable::notifySend()
{
	Array<Notification> notifications;

	// decide whom to send what
	foreach(const string *m, message_queue){
		//msg_write("send " + observable_name + ": " + queue[i]);
		foreach(ObserverRequest &r, requests){
			if ((r.message == m) or (r.message == &MESSAGE_ALL))
				if (r.observer->observer_enabled)
					notifications.add(Notification(r.observer, *m));
		}
	}

	message_queue.clear();

	// send
	foreach(Notification &n, notifications){
		if (DEBUG_MESSAGES)
			msg_write("send " + getName() + "/" + *n.message + "  >>  " + n.observer->getName());
		n.observer->onUpdate(this, *n.message);
	}
}


void Observable::notifyEnqueue(const string &message)
{
	// already enqueued?
	foreach(const string *m, message_queue)
		if (&message == m)
			return;

	// add
	message_queue.add(&message);
}

void Observable::notifyBegin()
{
	notify_level ++;
	//msg_write("notify ++");
}

void Observable::notifyEnd()
{
	notify_level --;
	//msg_write("notify --");
	if (notify_level == 0)
		notifySend();
}


void Observable::notify(const string &message)
{
	notifyEnqueue(message);
	if (notify_level == 0)
		notifySend();
}


