/*
 * ActionTrack__BufferSetChannels.cpp
 *
 *  Created on: Jul 8, 2018
 *      Author: michi
 */

#include "ActionTrack__BufferSetChannels.h"

#include "../../../Data/Track.h"

ActionTrack__BufferSetChannels::ActionTrack__BufferSetChannels(TrackLayer *_layer, int _index, int _channels)
{
	layer = _layer;
	index = _index;
	channels = _channels;
}

void *ActionTrack__BufferSetChannels::execute(Data *d)
{
	layer->buffers[index].c[1].exchange(temp);
	std::swap(layer->buffers[index].channels, channels);
	return NULL;
}

void ActionTrack__BufferSetChannels::undo(Data *d)
{
	execute(d);
}

