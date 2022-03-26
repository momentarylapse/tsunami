/*
 * ActionTrack__ShrinkBuffer.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include <assert.h>
#include "ActionTrack__ShrinkBuffer.h"
#include "../../../data/TrackLayer.h"
#include "../../../data/audio/AudioBuffer.h"

ActionTrack__ShrinkBuffer::ActionTrack__ShrinkBuffer(TrackLayer *l, int _index, int _length) {
	layer = l;
	index = _index;
	new_length = _length;
	old_length = 0;
	buf = AudioBuffer(0, l->channels);
}

void ActionTrack__ShrinkBuffer::undo(Data *d) {
	AudioBuffer &b = layer->buffers[index];

	// restore
	b.resize(old_length);
	b.set(buf, new_length, 1.0f);

	// clear temp data
	buf.clear();
}



void *ActionTrack__ShrinkBuffer::execute(Data *d) {
	AudioBuffer &b = layer->buffers[index];

	//msg_write(format("shrink %d   %d -> %d", index, b.num, new_length));

	assert(new_length < b.length);

	// copy data
	old_length = b.length;
	buf.resize(old_length - new_length);
	buf.set(b, -new_length, 1.0f);

	// shrink
	b.resize(new_length);

	return nullptr;
}


