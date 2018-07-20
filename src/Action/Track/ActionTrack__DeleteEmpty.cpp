/*
 * ActionTrack__DeleteEmpty.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionTrack__DeleteEmpty.h"
#include <assert.h>

ActionTrack__DeleteEmpty::ActionTrack__DeleteEmpty(Track *_track)
{
	index = _track->get_index();
	track = NULL;
}

ActionTrack__DeleteEmpty::~ActionTrack__DeleteEmpty()
{
	if (track)
		delete track;
}

void *ActionTrack__DeleteEmpty::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0 and index < a->tracks.num);
	Track *t = a->tracks[index];
	int num_buf = 0;
	int num_samples = 0;
	for (TrackLayer *l : t->layers){
		num_buf += l->buffers.num;
		num_samples += l->samples.num;
	}
	assert(num_buf == 0);
	assert(num_samples == 0);

	// TODO: no layers allowed?!?

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
	Song *a = dynamic_cast<Song*>(d);
	a->tracks.insert(track, index);
	a->notify(a->MESSAGE_ADD_TRACK);
	track = NULL;
}


