/*
 * ActionTrackAddBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackAddBar.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackAddBar::ActionTrackAddBar(Track *t, int _index, BarPattern &_bar)
{
	track_no = get_track_index(t);
	index = _index;
	bar = _bar;
}

ActionTrackAddBar::~ActionTrackAddBar()
{
}

void *ActionTrackAddBar::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);
	assert(index >= 0);
	assert(index <= t->bar.num);

	t->bar.insert(bar, index);
	t->Notify();

	return NULL;
}

void ActionTrackAddBar::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	Track *t = a->get_track(track_no);
	t->bar.erase(index);
	t->Notify();
}

