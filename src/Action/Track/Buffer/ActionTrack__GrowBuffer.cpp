/*
 * ActionTrackGrowBuffer.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrack__GrowBuffer.h"
#include "../../../Data/Track.h"
#include "../../../Data/Audio/AudioBuffer.h"

ActionTrack__GrowBuffer::ActionTrack__GrowBuffer(TrackLayer *l, int _index, int _new_length)
{
	layer = l;
	index = _index;
	new_length = _new_length;
	old_length = 0;
}

void *ActionTrack__GrowBuffer::execute(Data *d)
{
	AudioBuffer &b = layer->buffers[index];
	old_length = b.length;
	b.resize(new_length);

	return nullptr;
}



void ActionTrack__GrowBuffer::undo(Data *d)
{
	AudioBuffer &b = layer->buffers[index];
	b.resize(old_length);
}


