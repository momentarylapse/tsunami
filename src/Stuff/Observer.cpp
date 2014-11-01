/*
 * Observer.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "Observer.h"

Observer::Observer(const string &name)
{
	observer_name = name;
}

Observer::~Observer()
{
}



void Observer::subscribe(Observable *o, const string &message)
{
	o->addObserver(this, message);
}

void Observer::unsubscribe(Observable *o)
{
	o->removeObserver(this);
}

string Observer::getName()
{
	return observer_name;
}




ObserverWrapper::ObserverWrapper(void *_handler, void *_func) :
	Observer("Wrapper")
{
	handler = _handler;
	func = _func;
}

ObserverWrapper::~ObserverWrapper()
{}

void ObserverWrapper::onUpdate(Observable *o, const string &message)
{
	if (handler){
		typedef void mfunct(void *);
		mfunct *f = (mfunct*)func;
		f(handler);
	}else{
		typedef void funct();
		funct *f = (funct*)func;
		f();
	}
}

