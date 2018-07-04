/*
 * ActionTrackGrowBuffer.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "../../../Data/Track.h"
#include "ActionTrack__GrowBuffer.h"

ActionTrack__GrowBuffer::ActionTrack__GrowBuffer(TrackLayer *l, int _index, int _new_length)
{
	layer = l;
	index = _index;
	new_length = _new_length;
	old_length = 0;
}

void *ActionTrack__GrowBuffer::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	AudioBuffer &b = layer->buffers[index];
	old_length = b.length;
	b.resize(new_length);

	return NULL;
}



void ActionTrack__GrowBuffer::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	AudioBuffer &b = layer->buffers[index];
	b.resize(old_length);
}


