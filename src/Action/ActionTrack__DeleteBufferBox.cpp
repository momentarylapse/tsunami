/*
 * ActionTrack__DeleteBufferBox.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionTrack__DeleteBufferBox.h"
#include <assert.h>

ActionTrack__DeleteBufferBox::ActionTrack__DeleteBufferBox(Track *t, int _index)
{
	get_track_sub_index(t, track_no, sub_no);
	index = _index;
}

ActionTrack__DeleteBufferBox::~ActionTrack__DeleteBufferBox()
{
}

void ActionTrack__DeleteBufferBox::redo(Data *d)
{
	execute(d);
}



void ActionTrack__DeleteBufferBox::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no, sub_no);
	//BufferBox &b = t->buffer[index];

	t->buffer.insert(buf, index);

	buf.clear();
}



void *ActionTrack__DeleteBufferBox::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no, sub_no);
	BufferBox &b = t->buffer[index];

	assert(index >= 0 && index < t->buffer.num);

	// save data
	buf = b;

	// delete
	t->buffer.erase(index);
	return NULL;
}


