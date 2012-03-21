/*
 * Observer.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef OBSERVER_H_
#define OBSERVER_H_

#include "Observable.h"
#include "../lib/file/file.h"

class Observable;

class Observer
{
public:
	Observer();
	virtual ~Observer();

	void Subscribe(Observable *o);
	void Subscribe(Observable *o, const string &message);
	void Unsubscribe(Observable *o);

	virtual void OnUpdate(Observable *o) = 0;
};

#endif /* OBSERVER_H_ */
