/*
 * ActionTrack__BufferSetChannels.cpp
 *
 *  Created on: Jul 8, 2018
 *      Author: michi
 */

#include "ActionTrack__BufferSetChannels.h"
#include "../../../data/TrackLayer.h"
#include "../../../data/audio/AudioBuffer.h"

ActionTrack__BufferSetChannels::ActionTrack__BufferSetChannels(TrackLayer *_layer, int _index, int _channels)
{
	layer = _layer;
	index = _index;
	channels = _channels;
	if (channels == 2)
		temp = layer->buffers[index].c[0];
}

void *ActionTrack__BufferSetChannels::execute(Data *d)
{
	layer->buffers[index].c[1].exchange(temp);
	std::swap(layer->buffers[index].channels, channels);
	layer->buffers[index].invalidate_peaks(Range::ALL);
	return nullptr;
}

void ActionTrack__BufferSetChannels::undo(Data *d)
{
	execute(d);
}

