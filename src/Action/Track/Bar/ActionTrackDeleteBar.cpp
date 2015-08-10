/*
 * ActionTrackDeleteBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackDeleteBar.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackDeleteBar::ActionTrackDeleteBar(Track *t, int _index, bool _affect_midi)
{
	track_no = get_track_index(t);
	index = _index;
	affect_midi = _affect_midi;
}

ActionTrackDeleteBar::~ActionTrackDeleteBar()
{
}

void *ActionTrackDeleteBar::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);
	assert(index >= 0);
	assert(index < t->bars.num);

	bar = t->bars[index];
	t->bars.erase(index);
	t->notify();

	return NULL;
}

void ActionTrackDeleteBar::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);
	assert(t);
	assert(index >= 0);
	assert(index <= t->bars.num);

	t->bars.insert(bar, index);
	t->notify();
}

