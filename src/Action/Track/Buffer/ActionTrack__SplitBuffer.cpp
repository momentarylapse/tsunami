/*
 * ActionTrack__SplitBuffer.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include <assert.h>

#include "ActionTrack__SplitBuffer.h"

ActionTrack__SplitBuffer::ActionTrack__SplitBuffer(TrackLayer *l, int _index, int _offset)
{
	layer = l;
	index = _index;
	offset = _offset;
}



void ActionTrack__SplitBuffer::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	AudioBuffer &b = layer->buffers[index];
	AudioBuffer &b2 = layer->buffers[index + 1];

	// transfer data
	b.resize(b.length + b2.length);
	b.set(b2, offset, 1.0f);

	// delete
	layer->buffers.erase(index + 1);
}



void *ActionTrack__SplitBuffer::execute(Data *d)
{
	//msg_write(format("cut %d   at %d", index, offset));
	Song *a = dynamic_cast<Song*>(d);

	assert(offset > 0);
	assert(offset < (layer->buffers[index].length - 1));

	// create new
	AudioBuffer dummy;
	layer->buffers.insert(dummy, index + 1);

	AudioBuffer &b = layer->buffers[index];
	AudioBuffer &b2 = layer->buffers[index + 1];

	// new position
	b2.offset = b.offset + offset;

	// transfer data
	b2.resize(b.length - offset);
	b2.set(b, -offset, 1.0f);
	b.resize(offset);
	return &b;
}


