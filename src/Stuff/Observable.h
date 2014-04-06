/*
 * Observable.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef OBSERVABLE_H_
#define OBSERVABLE_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"

class Observer;
struct ObserverRequest;

class Observable : public HuiEventHandler
{
public:
	Observable(const string &name);
	virtual ~Observable();

	static const string MESSAGE_CHANGE;
	static const string MESSAGE_DELETE;
	static const string MESSAGE_ALL;

	void AddObserver(Observer *o, const string &message = MESSAGE_ALL);
	void AddWrappedObserver(void *handler, void *func);
	void RemoveObserver(Observer *o);
	void RemoveWrappedObserver(void *handler);
	string GetName();

	void NotifyBegin();
	void Notify(const string &message = MESSAGE_CHANGE);
	void NotifyEnd();
private:
	void NotifyEnqueue(const string &message);
	void NotifySend();

private:
	string observable_name;

	// observers
	Array<ObserverRequest> requests;

	// current notifies
	Array<const string*> message_queue;
	int notify_level;
};

#endif /* OBSERVABLE_H_ */
