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
#include "../../Data/Track.h"
#include <assert.h>

ActionTrackCreateBuffers::ActionTrackCreateBuffers(Track *t, int level_no, const Range &r)
{
	TrackLevel &l = t->level[level_no];

	// is <pos> inside a buffer?
	// last buffer before <pos>?
	int n_pos = -1;
	int n_before = -1;
	foreachi(l.buffer, b, i){
		if ((r.offset >= b.offset) && (r.offset <= b.offset + b.num))
			n_pos = i;
		if (r.offset >= b.offset)
			n_before = i;
	}
//	msg_write("get buf");
//	msg_write(n_pos);
//	msg_write(n_before);

	msg_write(n_pos);
	if (n_pos >= 0){
		msg_write("inside");

		// use base buffers
		BufferBox *b = &l.buffer[n_pos];

		// too small?
		if (r.end() > b->offset + b->num){
			msg_write("small");
			AddSubAction(new ActionTrack__GrowBufferBox(t, level_no, n_pos, r.end() - b->offset), t->root);
		}
	}else{

		// insert new buffers
		n_pos = n_before + 1;
		AddSubAction(new ActionTrack__AddBufferBox(t, level_no, n_pos, r), t->root);
	}

	// collision???  -> absorb
	for (int i=l.buffer.num-1;i>n_pos;i--)
		if (l.buffer[i].offset <= r.end()){
			msg_write("col");
			AddSubAction(new ActionTrack__AbsorbBufferBox(t, level_no, n_pos, i), t->root);
		}

//	for (int i=0;i<t->buffer_r.num;i++)
//		msg_write(format("%d   %d  %s", t->buffer_r[i].offset, t->buffer_r[i].b.num, (i == n_pos) ? "(*)" : ""));

	// return subarray (as reference...)
	//buf.set_as_ref(*b, pos - b->offset, length);
}

ActionTrackCreateBuffers::~ActionTrackCreateBuffers()
{
}

