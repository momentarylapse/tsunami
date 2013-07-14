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
	name = _name;
}

ActionTrackEditName::~ActionTrackEditName()
{
}

void *ActionTrackEditName::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	string temp = name;
	name = t->name;
	t->name = temp;

	return NULL;
}

void ActionTrackEditName::undo(Data *d)
{
	execute(d);
}
