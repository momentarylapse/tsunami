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

struct ObserverRequest
{
	ObserverRequest(){}
	ObserverRequest(Observer *o, const string &_message);
	Observer* observer;
	const string *message;
};

class Observable : public hui::EventHandler
{
public:
	Observable(const string &name);
	virtual ~Observable();

	static const string MESSAGE_CHANGE;
	static const string MESSAGE_DELETE;
	static const string MESSAGE_ALL;

	void addObserver(Observer *o, const string &message = MESSAGE_ALL);
	void addWrappedObserver(void *handler, void *func);
	void removeObserver(Observer *o);
	void removeWrappedObserver(void *handler);
	string getName();

	void notifyBegin();
	void notify(const string &message = MESSAGE_CHANGE);
	void notifyEnd();
private:
	void notifyEnqueue(const string &message);
	void notifySend();

protected:
	void _observable_destruct_();

private:
	string observable_name;

	// observers
	Array<ObserverRequest> requests;

	// current notifies
	Array<const string*> message_queue;
	int notify_level;

	static const bool DEBUG_MESSAGES;
};

#endif /* OBSERVABLE_H_ */
