/*
 * Action.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Action.h"
#include "../lib/threads/Mutex.h"

Action::Action()
{
}

Action::~Action()
{
}

void Action::undo_and_notify(Data *d)
{
	d->mutex->lock();
	//d->notifyBegin();
	undo(d);
	//d->notify(d->MESSAGE_CHANGE);
	//d->notifyEnd();
	d->mutex->unlock();
}



void *Action::execute_and_notify(Data *d)
{
	d->mutex->lock();
	//d->notifyBegin();
	void *r = execute(d);
	//d->notify(d->MESSAGE_CHANGE);
	//d->notifyEnd();
	d->mutex->unlock();
	return r;
}



void Action::redo_and_notify(Data *d)
{
	d->mutex->lock();
	//d->notifyBegin();
	redo(d);
	//d->notify(d->MESSAGE_CHANGE);
	//d->notifyEnd();
	d->mutex->unlock();
}



// default behavior for redo...
void Action::redo(Data *d)
{
	execute(d);
}


