/*
 * Action.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Action.h"

Action::Action()
{
}

Action::~Action()
{
}

void Action::undo_and_notify(Data *d)
{
	d->notifyBegin();
	undo(d);
	d->notify(d->MESSAGE_CHANGE);
	d->notifyEnd();
}



void *Action::execute_and_notify(Data *d)
{
	d->notifyBegin();
	void *r = execute(d);
	d->notify(d->MESSAGE_CHANGE);
	d->notifyEnd();
	return r;
}



void Action::redo_and_notify(Data *d)
{
	d->notifyBegin();
	redo(d);
	d->notify(d->MESSAGE_CHANGE);
	d->notifyEnd();
}



// default behavior for redo...
void Action::redo(Data *d)
{
	execute(d);
}


