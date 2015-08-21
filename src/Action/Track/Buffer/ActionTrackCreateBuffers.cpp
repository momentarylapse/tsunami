/*
 * ActionTrackCreateBuffers.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrackCreateBuffers.h"
#include "ActionTrack__AddBufferBox.h"
#include "ActionTrack__GrowBufferBox.h"
#include "ActionTrack__AbsorbBufferBox.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackCreateBuffers::ActionTrackCreateBuffers(Track *t, int level_no, const Range &r)
{
	TrackLevel &l = t->levels[level_no];

	// is <pos> inside a buffer?
	// last buffer before <pos>?
	int n_pos = -1;
	int n_before = -1;
	foreachi(BufferBox &b, l.buffers, i){
		if ((r.offset >= b.offset) && (r.offset <= b.offset + b.num))
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
		BufferBox &b = l.buffers[n_pos];

		// too small?
		if (r.end() > b.offset + b.num)
			addSubAction(new ActionTrack__GrowBufferBox(t, level_no, n_pos, r.end() - b.offset), t->song);
	}else{

		// insert new buffers
		n_pos = n_before + 1;
		addSubAction(new ActionTrack__AddBufferBox(t, level_no, n_pos, r), t->song);
	}

	// collision???  -> absorb
	for (int i=l.buffers.num-1;i>n_pos;i--)
		if (l.buffers[i].offset <= r.end())
			addSubAction(new ActionTrack__AbsorbBufferBox(t, level_no, n_pos, i), t->song);

//	for (int i=0;i<t->buffer_r.num;i++)
//		msg_write(format("%d   %d  %s", t->buffer_r[i].offset, t->buffer_r[i].b.num, (i == n_pos) ? "(*)" : ""));

	// return subarray (as reference...)
	//buf.set_as_ref(*b, pos - b->offset, length);
}

ActionTrackCreateBuffers::~ActionTrackCreateBuffers()
{
}

