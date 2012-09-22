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
	foreach(Action *a, action)
		delete(a);
	action.clear();
}

void *ActionGroup::AddSubAction(Action *a, Data *d)
{
	action.add(a);
	return a->execute(d);
}

// to be overwritten by subclasses
void *ActionGroup::execute_return(Data *d)
{	return NULL;	}


void *ActionGroup::execute(Data *d)
{
	/*foreach(Action *a, action)
		a->execute(d);*/
	return execute_return(d);
}



void ActionGroup::undo(Data *d)
{
	foreachb(Action *a, action)
		a->undo(d);
}



void ActionGroup::redo(Data *d)
{
	foreach(Action *a, action)
		a->redo(d);
}


