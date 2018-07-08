/*
 * ActionTrack__AbsorbBuffer.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "../../../Data/Song.h"
#include "ActionTrack__AbsorbBuffer.h"

ActionTrack__AbsorbBuffer::ActionTrack__AbsorbBuffer(TrackLayer *l, int _dest, int _src)
{
	layer = l;
	dest = _dest;
	src = _src;
	src_offset = 0;
	src_length = 0;
}

ActionTrack__AbsorbBuffer::~ActionTrack__AbsorbBuffer()
{
}

void *ActionTrack__AbsorbBuffer::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);


	AudioBuffer &b_src  = layer->buffers[src];
	AudioBuffer &b_dest = layer->buffers[dest];
	dest_old_length = b_dest.length;
	int new_size = b_src.offset + b_src.length - b_dest.offset;
	if (new_size > b_dest.length)
		b_dest.resize(new_size);

	src_offset = layer->buffers[src].offset;
	src_length = layer->buffers[src].length;
	b_dest.set(b_src, b_src.offset - b_dest.offset, 1.0f);

	layer->buffers.erase(src);

	return NULL;
}



void ActionTrack__AbsorbBuffer::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	//msg_todo("absorb undo...");
	AudioBuffer dummy;
	dummy.clear_x(layer->channels);
	layer->buffers.insert(dummy, src);
	AudioBuffer &b_src  = layer->buffers[src];
	AudioBuffer &b_dest = layer->buffers[dest];
	b_src.offset = src_offset;
	b_src.resize(src_length);

	b_src.set(b_dest, b_dest.offset - b_src.offset, 1.0f);
	b_dest.resize(dest_old_length);
}


