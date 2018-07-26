/*
 * ActionTrackEditName.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditName.h"
#include "../../../Data/Track.h"

ActionTrackEditName::ActionTrackEditName(Track *t, const string &_name)
{
	track = t;
	old_value = t->name;
	new_value = _name;
}

void *ActionTrackEditName::execute(Data *d)
{
	track->name = new_value;
	track->notify();

	return nullptr;
}

void ActionTrackEditName::undo(Data *d)
{
	track->name = old_value;
	track->notify();
}


bool ActionTrackEditName::mergable(Action *a)
{
	ActionTrackEditName *aa = dynamic_cast<ActionTrackEditName*>(a);
	if (!aa)
		return false;
	return (aa->track == track);
}
