/*
 * ActionTrack__ShrinkBufferBox.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionTrack__ShrinkBufferBox.h"
#include <assert.h>

ActionTrack__ShrinkBufferBox::ActionTrack__ShrinkBufferBox(Track *t, int _index, int _length)
{
	get_track_sub_index(t, track_no, sub_no);
	index = _index;
	new_length = _length;
}

ActionTrack__ShrinkBufferBox::~ActionTrack__ShrinkBufferBox()
{
}

void ActionTrack__ShrinkBufferBox::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no, sub_no);
	BufferBox &b = t->buffer[index];

	// restore
	b.resize(old_length);
	b.set(buf, new_length, 1.0f);

	// clear temp data
	buf.clear();
}



void *ActionTrack__ShrinkBufferBox::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no, sub_no);
	BufferBox &b = t->buffer[index];

	//msg_write(format("shrink %d   %d -> %d", index, b.num, new_length));

	assert(new_length < b.num);

	// copy data
	old_length = b.num;
	buf.resize(old_length - new_length);
	buf.set(b, -new_length, 1.0f);

	// shrink
	b.resize(new_length);

	return &t->buffer[index];
}


