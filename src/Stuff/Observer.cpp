/*
 * Observer.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "Observer.h"

Observer::Observer()
{
}

Observer::~Observer()
{
}



void Observer::Subscribe(Observable *o, const string &message)
{
	o->AddObserver(this, message);
}

void Observer::Subscribe(Observable *o)
{
	o->AddObserver(this);
}

void Observer::Unsubscribe(Observable *o)
{
	o->RemoveObserver(this);
}




ObserverWrapper::ObserverWrapper(void *_handler, void *_func)
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

