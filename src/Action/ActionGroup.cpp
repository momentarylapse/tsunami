/*
 * ActionGroup.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionGroup.h"

ActionGroup::ActionGroup()
{
}

ActionGroup::~ActionGroup()
{
	for (Action *a: action)
		delete(a);
	action.clear();
}

void *ActionGroup::addSubAction(Action *a, Data *d)
{
	void *r = a->execute(d);
	if (!a->is_trivial())
		action.add(a);
	return r;
}

// to be overwritten by subclasses
void *ActionGroup::execute_return(Data *d)
{	return NULL;	}


void *ActionGroup::execute(Data *d)
{
	build(d);
	return execute_return(d);
}



void ActionGroup::undo(Data *d)
{
	foreachb(Action *a, action)
		a->undo(d);
}



void ActionGroup::redo(Data *d)
{
	for (Action *a: action)
		a->redo(d);
}

bool ActionGroup::is_trivial()
{
	for (Action *a: action)
		if (!a->is_trivial())
			return false;
	return true;
}

