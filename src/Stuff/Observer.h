/*
 * Observer.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef OBSERVER_H_
#define OBSERVER_H_

#include "Observable.h"
#include "../lib/base/base.h"

//class Observable;

class Observer
{
public:
	Observer(const string &name);
	virtual ~Observer();

	void subscribe(Observable *o, const string &message = Observable::MESSAGE_ALL);
	void unsubscribe(Observable *o);

	string getName();

	virtual void onUpdate(Observable *o, const string &message) = 0;

private:
	string observer_name;
};

class ObserverWrapper : public Observer
{
public:
	ObserverWrapper(void *handler, void *func);
	virtual ~ObserverWrapper();

	virtual void onUpdate(Observable *o, const string &message);

	void *handler, *func;
};

#endif /* OBSERVER_H_ */
