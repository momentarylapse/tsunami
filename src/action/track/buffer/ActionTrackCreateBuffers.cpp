/*
 * ActionTrackCreateBuffers.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrackCreateBuffers.h"
#include <assert.h>
#include "ActionTrack__AbsorbBuffer.h"
#include "ActionTrack__AddBuffer.h"
#include "ActionTrack__GrowBuffer.h"
#include "../../../data/TrackLayer.h"
#include "../../../data/Song.h"
#include "../../../data/audio/AudioBuffer.h"

ActionTrackCreateBuffers::ActionTrackCreateBuffers(TrackLayer *l, const Range &_r)
{
	layer = l;
	r = _r;
}

void ActionTrackCreateBuffers::build(Data *d)
{
	// is <pos> inside a buffer?
	// last buffer before <pos>?
	int n_pos = -1;
	int n_before = -1;
	foreachi(AudioBuffer &b, layer->buffers, i){
		if ((r.offset >= b.offset) and (r.offset <= b.offset + b.length))
			n_pos = i;
		if (r.offset >= b.offset)
			n_before = i;
	}
//	msg_write("get buf");
//	msg_write(n_pos);
//	msg_write(n_before);

	if (n_pos >= 0){
		//msg_write("inside");

		// use base buffers
		AudioBuffer &b = layer->buffers[n_pos];

		// too small?
		if (r.end() > b.offset + b.length)
			add_sub_action(new ActionTrack__GrowBuffer(layer, n_pos, r.end() - b.offset), d);
	}else{

		// insert new buffers
		n_pos = n_before + 1;
		add_sub_action(new ActionTrack__AddBuffer(layer, n_pos, r), d);
	}

	// collision???  -> absorb
	for (int i=layer->buffers.num-1;i>n_pos;i--)
		if (layer->buffers[i].offset <= r.end())
			add_sub_action(new ActionTrack__AbsorbBuffer(layer, n_pos, i), d);

//	for (int i=0;i<t->buffer_r.num;i++)
//		msg_write(format("%d   %d  %s", t->buffer_r[i].offset, t->buffer_r[i].b.num, (i == n_pos) ? "(*)" : ""));

	// return subarray (as reference...)
	//buf.set_as_ref(*b, pos - b->offset, length);
}

