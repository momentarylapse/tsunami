/*
 * Observable.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "Observable.h"
#include "Observer.h"

ObserverRequest::ObserverRequest(Observer *o, const string &_message)
{
	observer = o;
	message = _message;
}

Observable::Observable(const string &name)
{
	notify_level = 0;
	observable_name = name;
}

Observable::~Observable()
{
	requests.clear();
}

void Observable::AddObserver(Observer *o, const string &message)
{
	requests.add(ObserverRequest(o, message));
}

void Observable::AddObserver(Observer *o)
{	AddObserver(o, "");	}



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
	AddObserver(o);
}

void Observable::RemoveWrappedObserver(void* handler)
{
	foreachi(ObserverRequest &r, requests, i)
		if (dynamic_cast<ObserverWrapper*>(r.observer)){
			if (dynamic_cast<ObserverWrapper*>(r.observer)->handler == handler){
				requests.erase(i);
				break;
			}
		}
}



string Observable::GetName()
{	return observable_name;	}



void Observable::NotifySend()
{
	Array<string> queue = message_queue;
	message_queue.clear();

	// send
	for (int i=0;i<queue.num;i++)
		for (int j=0;j<requests.num;j++){
			//msg_write("send " + observable_name + ": " + queue[i]);
			if ((requests[j].message == queue[i]) or (requests[j].message.num == 0)){
				requests[j].observer->OnUpdate(this, queue[i]);
			}
	}
}


void Observable::NotifyEnqueue(const string &message)
{
	// already enqueued?
	foreach(string &m, message_queue)
		if (message == m)
			return;

	// add
	message_queue.add(message);
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


