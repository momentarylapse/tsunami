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


