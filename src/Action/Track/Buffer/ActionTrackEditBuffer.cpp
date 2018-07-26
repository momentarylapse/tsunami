/*
 * ActionTrackEditBuffer.cpp
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#include "ActionTrackEditBuffer.h"
#include <assert.h>
#include "../../ActionManager.h"
#include "../../../Data/Track.h"
#include "../../../Data/Song.h"
#include "../../../Data/Audio/AudioBuffer.h"

ActionTrackEditBuffer::ActionTrackEditBuffer(TrackLayer *l, Range _range)
{
	// prepare...
	range = _range;
	layer = l;

	index = -1;
	foreachi(AudioBuffer &buf, layer->buffers, i)
		if (buf.range().covers(range))
			index = i;
	assert(index >= 0);

	if (!l->track->song->action_manager->isEnabled())
		return;

	// save old data
	AudioBuffer b;
	layer->readBuffers(b, range, true);
	box = AudioBuffer(b.length, l->channels);
	box.set(b, 0, 1.0f);
}



void ActionTrackEditBuffer::undo(Data *d)
{
	layer->buffers[index].invalidate_peaks(range);

	AudioBuffer b;
	layer->readBuffers(b, range, true);
	box.swap_value(b);
}

void ActionTrackEditBuffer::redo(Data *d)
{
	undo(d);
}

void *ActionTrackEditBuffer::execute(Data *d)
{
	layer->buffers[index].invalidate_peaks(range);

	// nothing to do...
	return nullptr;
}

