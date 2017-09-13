/*
 * ActionTrackGrowBuffer.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "../../../Data/Track.h"
#include "ActionTrack__GrowBuffer.h"

ActionTrack__GrowBuffer::ActionTrack__GrowBuffer(Track *t, int _level_no, int _index, int _new_length)
{
	track_no = get_track_index(t);
	level_no = _level_no;
	index = _index;
	new_length = _new_length;
	old_length = 0;
}

void *ActionTrack__GrowBuffer::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	AudioBuffer &b = t->layers[level_no].buffers[index];
	old_length = b.length;
	b.resize(new_length);

	return NULL;
}



void ActionTrack__GrowBuffer::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	AudioBuffer &b = t->layers[level_no].buffers[index];
	b.resize(old_length);
}


