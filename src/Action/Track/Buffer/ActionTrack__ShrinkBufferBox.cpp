/*
 * ActionTrack__ShrinkBufferBox.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionTrack__ShrinkBufferBox.h"
#include <assert.h>

ActionTrack__ShrinkBufferBox::ActionTrack__ShrinkBufferBox(Track *t, int _level_no, int _index, int _length)
{
	track_no = get_track_index(t);
	level_no = _level_no;
	index = _index;
	new_length = _length;
}

ActionTrack__ShrinkBufferBox::~ActionTrack__ShrinkBufferBox()
{
}

void ActionTrack__ShrinkBufferBox::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	BufferBox &b = t->levels[level_no].buffers[index];

	// restore
	b.resize(old_length);
	b.set(buf, new_length, 1.0f);

	// clear temp data
	buf.clear();
}



void *ActionTrack__ShrinkBufferBox::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	BufferBox &b = t->levels[level_no].buffers[index];

	//msg_write(format("shrink %d   %d -> %d", index, b.num, new_length));

	assert(new_length < b.length);

	// copy data
	old_length = b.length;
	buf.resize(old_length - new_length);
	buf.set(b, -new_length, 1.0f);

	// shrink
	b.resize(new_length);

	return &t->levels[level_no].buffers[index];
}


