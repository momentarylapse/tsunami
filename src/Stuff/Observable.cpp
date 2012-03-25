/*
 * Observable.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "Observable.h"
#include "Observer.h"

Observable::Observable(const string &name)
{
	notify_level = 0;
	observable_name = name;
}

Observable::~Observable()
{
	observer.clear();
}

void Observable::AddObserver(Observer *o, const string &message)
{
	observer.add(o);
	observer_message.add(message);
}

void Observable::AddObserver(Observer *o)
{	AddObserver(o, "");	}



void Observable::RemoveObserver(Observer *o)
{
	foreachi(observer, obs, i)
		if (obs == o){
			observer.erase(i);
			observer_message.erase(i);
			break;
		}
}



string Observable::GetName()
{	return observable_name;	}



string Observable::GetMessage()
{	return cur_message;	}


void Observable::NotifySend()
{
	// send
	foreach(message_queue, m){
		cur_message = m;
		//msg_write("send " + observable_name + ": " + m);
		foreachi(observer, o, i)
			if ((observer_message[i] == m) or (observer_message[i].num == 0))
				o->OnUpdate(this);
	}

	// clear queue
	message_queue.clear();
}


void Observable::NotifyEnqueue(const string &message)
{
	// already enqueued?
	foreach(message_queue, m)
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

