/*
 * ActionTrackMoveBuffer.cpp
 *
 *  Created on: Sep 24, 2020
 *      Author: michi
 */

#include "ActionTrackMoveBuffer.h"
#include <assert.h>
#include "../../../data/TrackLayer.h"
#include "../../../data/audio/AudioBuffer.h"

namespace tsunami {

ActionTrackMoveBuffer::ActionTrackMoveBuffer(TrackLayer *l, int _index, int _shift) {
	layer = l;
	index = _index;
	shift = _shift;
}


void ActionTrackMoveBuffer::undo(Data *d) {
	AudioBuffer &b = layer->buffers[index];
	b.offset -= shift;
}



void *ActionTrackMoveBuffer::execute(Data *d) {
	assert(index > 0);
	assert(index < layer->buffers.num);
	AudioBuffer &b = layer->buffers[index];
	b.offset += shift;
	return nullptr;
}

}




