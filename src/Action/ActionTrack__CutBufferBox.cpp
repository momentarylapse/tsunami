/*
 * ActionTrack__CutBufferBox.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionTrack__CutBufferBox.h"
#include <assert.h>

ActionTrack__CutBufferBox::ActionTrack__CutBufferBox(Track *t, int _index, int _offset)
{
	get_track_sub_index(t, track_no, sub_no);
	index = _index;
	offset = _offset;
}

ActionTrack__CutBufferBox::~ActionTrack__CutBufferBox()
{
}

void ActionTrack__CutBufferBox::redo(Data *d)
{
	execute(d);
}



void ActionTrack__CutBufferBox::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no, sub_no);
	BufferBox &b = t->buffer[index];
	BufferBox &b2 = t->buffer[index + 1];

	// transfer data
	b.resize(b.num + b2.num);
	b.set(b2, offset, 1.0f);

	// delete
	t->buffer.erase(index + 1);
}



void *ActionTrack__CutBufferBox::execute(Data *d)
{
	//msg_write(format("cut %d   at %d", index, offset));
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no, sub_no);

	assert(offset > 0 && offset < (t->buffer[index].num - 1));

	// create new
	BufferBox dummy;
	t->buffer.insert(dummy, index + 1);

	BufferBox &b = t->buffer[index];
	BufferBox &b2 = t->buffer[index + 1];

	// new position
	b2.offset = b.offset + offset;

	// transfer data
	b2.resize(b.num - offset);
	b2.set(b, -offset, 1.0f);
	b.resize(offset);
	return &b;
}


