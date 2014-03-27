/*
 * ActionTrackEditMuted.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditMuted.h"
#include "../../../Data/Track.h"

ActionTrackEditMuted::ActionTrackEditMuted(Track *t, bool _muted)
{
	track_no = get_track_index(t);
	muted = _muted;
}

ActionTrackEditMuted::~ActionTrackEditMuted()
{
}

void *ActionTrackEditMuted::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	bool temp = muted;
	muted = t->muted;
	t->muted = temp;
	t->Notify("Change");

	return NULL;
}

void ActionTrackEditMuted::undo(Data *d)
{
	execute(d);
}
