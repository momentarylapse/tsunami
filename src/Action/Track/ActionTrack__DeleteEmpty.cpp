/*
 * ActionTrack__DeleteEmpty.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionTrack__DeleteEmpty.h"
#include <assert.h>

ActionTrack__DeleteEmpty::ActionTrack__DeleteEmpty(int _index)
{
	index = _index;
}

ActionTrack__DeleteEmpty::~ActionTrack__DeleteEmpty()
{
}

void *ActionTrack__DeleteEmpty::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0 && index < a->tracks.num);
	Track *t = a->tracks[index];
	int num_buf = 0;
	foreach(TrackLevel &l, t->levels)
		num_buf += l.buffers.num;
	assert(num_buf == 0);
	assert(t->samples.num == 0);

	// save data
	track = t;

	// delete
	track->notify(track->MESSAGE_DELETE);
	a->tracks.erase(index);
	a->notify(a->MESSAGE_DELETE_TRACK);
	return NULL;
}



void ActionTrack__DeleteEmpty::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	a->tracks.insert(track, index);
	a->notify(a->MESSAGE_ADD_TRACK);
}


