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

class Observable;

class Observer
{
public:
	Observer();
	virtual ~Observer();

	void Subscribe(Observable *o);
	void Subscribe(Observable *o, const string &message);
	void Unsubscribe(Observable *o);

	virtual void OnUpdate(Observable *o, const string &message) = 0;
};

class ObserverWrapper : public Observer
{
public:
	ObserverWrapper(void *handler, void *func);
	virtual ~ObserverWrapper();

	virtual void OnUpdate(Observable *o, const string &message);

	void *handler, *func;
};

#endif /* OBSERVER_H_ */
