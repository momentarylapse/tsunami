/*
 * Observable.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef OBSERVABLE_H_
#define OBSERVABLE_H_

#include "Observer.h"
#include "../lib/base/base.h"
#include "../lib/hui/hui.h"

class Observer;

struct ObserverRequest
{
	ObserverRequest(){}
	ObserverRequest(Observer *o, const string &message);
	Observer* observer;
	string message;
};

class Observable : public HuiEventHandler
{
public:
	Observable(const string &name);
	virtual ~Observable();

	void AddObserver(Observer *o);
	void AddObserver(Observer *o, const string &message);
	void AddWrappedObserver(void *handler, void *func);
	void RemoveObserver(Observer *o);
	void RemoveWrappedObserver(void *handler);
	string GetName();

	void NotifyBegin();
	void Notify(const string &message);
	void NotifyEnd();
private:
	void NotifyEnqueue(const string &message);
	void NotifySend();

private:
	string observable_name;

	// observers
	Array<ObserverRequest> requests;

	// current notifies
	Array<string> message_queue;
	int notify_level;
};

#endif /* OBSERVABLE_H_ */
