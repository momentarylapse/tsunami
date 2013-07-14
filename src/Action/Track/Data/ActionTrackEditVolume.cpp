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
	volume = _volume;
}

ActionTrackEditVolume::~ActionTrackEditVolume()
{
}

void *ActionTrackEditVolume::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	float temp = volume;
	volume = t->volume;
	t->volume = temp;

	return NULL;
}

void ActionTrackEditVolume::undo(Data *d)
{
	execute(d);
}
