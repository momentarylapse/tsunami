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



void Observer::Subscribe(Observable *o, const string &message)
{
	o->AddObserver(this, message);
}

void Observer::Unsubscribe(Observable *o)
{
	o->RemoveObserver(this);
}

string Observer::GetName()
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

void ObserverWrapper::OnUpdate(Observable *o, const string &message)
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

