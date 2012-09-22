/*
 * ActionAudio__DeleteTrack.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionAudio__DeleteTrack.h"
#include <assert.h>

ActionAudio__DeleteTrack::ActionAudio__DeleteTrack(int _index)
{
	index = _index;
}

ActionAudio__DeleteTrack::~ActionAudio__DeleteTrack()
{
}

void *ActionAudio__DeleteTrack::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0 && index < a->track.num);
	Track &t = a->track[index];
	int num_buf = 0;
	foreach(TrackLevel &l, t.level)
		num_buf += l.buffer.num;
	assert(num_buf == 0);
	assert(t.sub.num == 0);

	// save data
	track = t;

	// delete
	a->track.erase(index);
	return NULL;
}



void ActionAudio__DeleteTrack::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	a->track.insert(track, index);
}


