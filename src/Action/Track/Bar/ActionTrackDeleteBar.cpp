/*
 * ActionTrackDeleteBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackDeleteBar.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackDeleteBar::ActionTrackDeleteBar(Track *t, int _index)
{
	track_no = get_track_index(t);
	index = _index;
}

ActionTrackDeleteBar::~ActionTrackDeleteBar()
{
}

void *ActionTrackDeleteBar::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);
	assert(index >= 0);
	assert(index < t->bar.num);

	bar = t->bar[index];
	t->bar.erase(index);
	t->Notify("Change");

	return NULL;
}

void ActionTrackDeleteBar::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);
	assert(t);
	assert(index >= 0);
	assert(index <= t->bar.num);

	t->bar.insert(bar, index);
	t->Notify("Change");
}

