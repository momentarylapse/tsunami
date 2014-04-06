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
	Notify(MESSAGE_DELETE);
}

void Observable::AddObserver(Observer *o, const string &message)
{
	requests.add(ObserverRequest(o, message));
}

void Observable::RemoveObserver(Observer *o)
{
	for (int i=requests.num-1; i>=0; i--)
		if (requests[i].observer == o){
			requests.erase(i);
		}
}

void Observable::AddWrappedObserver(void* handler, void* func)
{
	Observer *o = new ObserverWrapper(handler, func);
	AddObserver(o, MESSAGE_ALL);
}

void Observable::RemoveWrappedObserver(void* handler)
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



string Observable::GetName()
{	return observable_name;	}

void Observable::NotifySend()
{
	Array<Notification> notifications;

	// decide whom to send what
	foreach(const string *m, message_queue){
		//msg_write("send " + observable_name + ": " + queue[i]);
		foreach(ObserverRequest &r, requests){
			if ((r.message == m) or (r.message == &MESSAGE_ALL))
				notifications.add(Notification(r.observer, *m));
		}
	}

	message_queue.clear();

	// send
	foreach(Notification &n, notifications){
		//msg_write("send " + GetName() + "/" + *n.message + "  >>  " + n.observer->GetName());
		n.observer->OnUpdate(this, *n.message);
	}
}


void Observable::NotifyEnqueue(const string &message)
{
	// already enqueued?
	foreach(const string *m, message_queue)
		if (&message == m)
			return;

	// add
	message_queue.add(&message);
}

void Observable::NotifyBegin()
{
	notify_level ++;
	//msg_write("notify ++");
}

void Observable::NotifyEnd()
{
	notify_level --;
	//msg_write("notify --");
	if (notify_level == 0)
		NotifySend();
}


void Observable::Notify(const string &message)
{
	NotifyEnqueue(message);
	if (notify_level == 0)
		NotifySend();
}


