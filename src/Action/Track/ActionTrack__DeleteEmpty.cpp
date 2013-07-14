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
	assert(index >= 0 && index < a->track.num);
	Track *t = a->track[index];
	int num_buf = 0;
	foreach(TrackLevel &l, t->level)
		num_buf += l.buffer.num;
	assert(num_buf == 0);
	assert(t->sample.num == 0);

	// save data
	track = t;

	// delete
	a->track.erase(index);
	return NULL;
}



void ActionTrack__DeleteEmpty::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	a->track.insert(track, index);
}


