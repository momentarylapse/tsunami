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
	track_no = get_track_index(t);
	old_value = t->name;
	new_value = _name;
}

ActionTrackEditName::~ActionTrackEditName()
{
}

void *ActionTrackEditName::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	t->name = new_value;
	t->Notify("Change");

	return NULL;
}

void ActionTrackEditName::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	t->name = old_value;
	t->Notify("Change");
}


bool ActionTrackEditName::mergable(Action *a)
{
	ActionTrackEditName *aa = dynamic_cast<ActionTrackEditName*>(a);
	if (!aa)
		return false;
	return (aa->track_no == track_no);
}
