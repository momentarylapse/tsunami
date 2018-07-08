/*
 * ActionTrack__AddBuffer.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "../../../Data/Song.h"
#include <assert.h>
#include "ActionTrack__AddBuffer.h"

ActionTrack__AddBuffer::ActionTrack__AddBuffer(TrackLayer *l, int _index, Range r)
{
	layer = l;
	index = _index;
	range = r;
}

void ActionTrack__AddBuffer::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	// should be zeroes at this point...
	layer->buffers.erase(index);
}



void *ActionTrack__AddBuffer::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	AudioBuffer dummy;
	dummy.clear_x(layer->channels);
	layer->buffers.insert(dummy, index);

	// reserve memory
	AudioBuffer &b = layer->buffers[index];
	b.offset = range.start();
	b.clear_x(layer->channels);
	b.resize(range.length);
	return &b;
}


