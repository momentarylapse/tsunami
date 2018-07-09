/*
 * ActionTrack__AddBuffer.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "../../../Data/Song.h"
#include <assert.h>
#include "ActionTrack__AddBuffer.h"

ActionTrack__AddBuffer::ActionTrack__AddBuffer(TrackLayer *l, int _index, const Range &r)
{
	layer = l;
	index = _index;
	range = r;
}

void ActionTrack__AddBuffer::undo(Data *d)
{
	// should be zeroes at this point...
	layer->buffers.erase(index);
}



void *ActionTrack__AddBuffer::execute(Data *d)
{
	AudioBuffer dummy(0, layer->channels);
	layer->buffers.insert(dummy, index);

	// reserve memory
	AudioBuffer &b = layer->buffers[index];
	b.offset = range.start();
	b.resize(range.length);
	return &b;
}


