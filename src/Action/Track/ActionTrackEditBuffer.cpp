/*
 * ActionTrackEditBuffer.cpp
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#include "ActionTrackEditBuffer.h"

ActionTrackEditBuffer::ActionTrackEditBuffer(Track *t, Range _range)
{
	// prepare...
	range = _range;
	get_track_sub_index(t, track_no, sub_no);

	// save old data
	BufferBox b = t->ReadBuffers(range);
	box.resize(b.num);
	box.set(b, 0, 1.0f);
}

ActionTrackEditBuffer::~ActionTrackEditBuffer()
{
}



void ActionTrackEditBuffer::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no, sub_no);

	BufferBox b = t->ReadBuffers(range);
	box.swap_value(b);
}

void *ActionTrackEditBuffer::execute(Data *d)
{
	// nothing to do...
	return NULL;
}



void ActionTrackEditBuffer::redo(Data *d)
{
	undo(d);
}


