/*
 * ActionTrack__CutBufferBox.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionTrack__CutBufferBox.h"
#include <assert.h>

ActionTrack__CutBufferBox::ActionTrack__CutBufferBox(Track *t, int _level_no, int _index, int _offset)
{
	track_no = get_track_index(t);
	index = _index;
	offset = _offset;
	level_no = _level_no;
}

ActionTrack__CutBufferBox::~ActionTrack__CutBufferBox()
{
}



void ActionTrack__CutBufferBox::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	BufferBox &b = t->levels[level_no].buffers[index];
	BufferBox &b2 = t->levels[level_no].buffers[index + 1];

	// transfer data
	b.resize(b.length + b2.length);
	b.set(b2, offset, 1.0f);

	// delete
	t->levels[level_no].buffers.erase(index + 1);
}



void *ActionTrack__CutBufferBox::execute(Data *d)
{
	//msg_write(format("cut %d   at %d", index, offset));
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	TrackLevel &l = t->levels[level_no];

	assert(offset > 0 && offset < (l.buffers[index].length - 1));

	// create new
	BufferBox dummy;
	l.buffers.insert(dummy, index + 1);

	BufferBox &b = l.buffers[index];
	BufferBox &b2 = l.buffers[index + 1];

	// new position
	b2.offset = b.offset + offset;

	// transfer data
	b2.resize(b.length - offset);
	b2.set(b, -offset, 1.0f);
	b.resize(offset);
	return &b;
}


