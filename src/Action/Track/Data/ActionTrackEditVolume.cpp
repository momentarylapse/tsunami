/*
 * ActionTrackEditVolume.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditVolume.h"
#include "../../../Data/Track.h"

ActionTrackEditVolume::ActionTrackEditVolume(Track *t, float _volume)
{
	track_no = get_track_index(t);
	old_value = t->volume;
	new_value = _volume;
}

ActionTrackEditVolume::~ActionTrackEditVolume()
{
}

void *ActionTrackEditVolume::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	t->volume = new_value;
	t->notify();

	return NULL;
}

void ActionTrackEditVolume::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	t->volume = old_value;
	t->notify();
}


bool ActionTrackEditVolume::mergable(Action *a)
{
	ActionTrackEditVolume *aa = dynamic_cast<ActionTrackEditVolume*>(a);
	if (!aa)
		return false;
	return (aa->track_no == track_no);
}
