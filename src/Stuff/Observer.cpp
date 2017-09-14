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
	observer_enabled = true;
}

Observer::~Observer()
{
}


string Observer::getName()
{
	return observer_name;
}

void Observer::allowNotification(bool allow)
{
	observer_enabled = allow;
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
		typedef void mfunct(void *, const string&);
		mfunct *f = (mfunct*)func;
		f(handler, message);
	}else{
		typedef void funct(const string&);
		funct *f = (funct*)func;
		f(message);
	}
}

