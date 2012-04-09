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
#include "../Data/Track.h"
#include <assert.h>

ActionTrackCreateBuffers::ActionTrackCreateBuffers(Track *t, int pos, int length)
{
	// is <pos> inside a buffer?
	// last buffer before <pos>?
	int n_pos = -1;
	int n_before = -1;
	for (int i=0;i<t->buffer.num;i++){
		if ((pos >= t->buffer[i].offset) && (pos <= t->buffer[i].offset + t->buffer[i].num))
			n_pos = i;
		if (pos >= t->buffer[i].offset)
			n_before = i;
	}
//	msg_write("get buf");
//	msg_write(n_pos);
//	msg_write(n_before);

	if (n_pos >= 0){
		//msg_write("inside");

		// use base buffers
		BufferBox *b = &t->buffer[n_pos];

		// too small?
		if (pos + length > b->offset + b->num)
			AddSubAction(new ActionTrack__GrowBufferBox(t, n_pos, pos - b->offset + length), t->root);
	}else{

		// insert new buffers
		n_pos = n_before + 1;
		AddSubAction(new ActionTrack__AddBufferBox(t, n_pos, pos, length), t->root);
	}

	// collision???  -> absorb
	for (int i=t->buffer.num-1;i>n_pos;i--)
		if (t->buffer[i].offset <= pos + length)
			AddSubAction(new ActionTrack__AbsorbBufferBox(t, n_pos, i), t->root);

//	for (int i=0;i<t->buffer_r.num;i++)
//		msg_write(format("%d   %d  %s", t->buffer_r[i].offset, t->buffer_r[i].b.num, (i == n_pos) ? "(*)" : ""));

	// return subarray (as reference...)
	//buf.set_as_ref(*b, pos - b->offset, length);
}

ActionTrackCreateBuffers::~ActionTrackCreateBuffers()
{
}

