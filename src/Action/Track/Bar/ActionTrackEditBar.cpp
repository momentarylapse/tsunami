/*
 * ActionTrackEditBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditBar.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackEditBar::ActionTrackEditBar(Track *t, int _index, BarPattern &_bar)
{
	track_no = get_track_index(t);
	index = _index;
	bar = _bar;
}

ActionTrackEditBar::~ActionTrackEditBar()
{
}

void *ActionTrackEditBar::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);
	assert(t);
	assert(index >= 0);
	assert(index < t->bar.num);

	BarPattern temp = bar;
	bar = t->bar[index];
	t->bar[index] = temp;

	return NULL;
}

void ActionTrackEditBar::undo(Data *d)
{
	execute(d);
}

