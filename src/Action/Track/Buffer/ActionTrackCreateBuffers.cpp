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

ActionTrackCreateBuffers::ActionTrackCreateBuffers(Track *t, int _level_no, const Range &_r)
{
	track_no = t->get_index();
	level_no = _level_no;
	r = _r;
}

void ActionTrackCreateBuffers::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	Track *t = s->tracks[track_no];

	TrackLayer &l = t->layers[level_no];

	// is <pos> inside a buffer?
	// last buffer before <pos>?
	int n_pos = -1;
	int n_before = -1;
	foreachi(BufferBox &b, l.buffers, i){
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
		BufferBox &b = l.buffers[n_pos];

		// too small?
		if (r.end() > b.offset + b.length)
			addSubAction(new ActionTrack__GrowBufferBox(t, level_no, n_pos, r.end() - b.offset), d);
	}else{

		// insert new buffers
		n_pos = n_before + 1;
		addSubAction(new ActionTrack__AddBufferBox(t, level_no, n_pos, r), d);
	}

	// collision???  -> absorb
	for (int i=l.buffers.num-1;i>n_pos;i--)
		if (l.buffers[i].offset <= r.end())
			addSubAction(new ActionTrack__AbsorbBufferBox(t, level_no, n_pos, i), d);

//	for (int i=0;i<t->buffer_r.num;i++)
//		msg_write(format("%d   %d  %s", t->buffer_r[i].offset, t->buffer_r[i].b.num, (i == n_pos) ? "(*)" : ""));

	// return subarray (as reference...)
	//buf.set_as_ref(*b, pos - b->offset, length);
}

