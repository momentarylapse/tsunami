/*
 * Action.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Action.h"

Action::Action()
{
	// TODO Auto-generated constructor stub

}

Action::~Action()
{
	// TODO Auto-generated destructor stub
}

void Action::undo_and_notify(Data *d)
{
	d->NotifyBegin();
	undo(d);
	d->PostActionUpdate();
	d->Notify(d->MESSAGE_CHANGE);
	d->NotifyEnd();
}



void *Action::execute_and_notify(Data *d)
{
	d->NotifyBegin();
	void *r = execute(d);
	d->PostActionUpdate();
	d->Notify(d->MESSAGE_CHANGE);
	d->NotifyEnd();
	return r;
}



void Action::redo_and_notify(Data *d)
{
	d->NotifyBegin();
	redo(d);
	d->PostActionUpdate();
	d->Notify(d->MESSAGE_CHANGE);
	d->NotifyEnd();
}



// default behavior for redo...
void Action::redo(Data *d)
{
	execute(d);
}


