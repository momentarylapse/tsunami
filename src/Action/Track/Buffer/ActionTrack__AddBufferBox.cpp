/*
 * ActionTrack__AddBufferBox.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrack__AddBufferBox.h"
#include "../../../Data/AudioFile.h"
#include <assert.h>

ActionTrack__AddBufferBox::ActionTrack__AddBufferBox(Track *t, int _level_no, int _index, Range r)
{
	get_track_sub_index(t, track_no, sub_no);
	index = _index;
	range = r;
	level_no = _level_no;
}

ActionTrack__AddBufferBox::~ActionTrack__AddBufferBox()
{
}

void ActionTrack__AddBufferBox::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no, sub_no);

	// should be zeroes at this point...
	t->level[level_no].buffer.erase(index);
}



void *ActionTrack__AddBufferBox::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no, sub_no);
	assert(t && "AddBufferBox.execute");

	BufferBox dummy;
	t->level[level_no].buffer.insert(dummy, index);

	// reserve memory
	BufferBox &b = t->level[level_no].buffer[index];
	b.offset = range.start();
	b.resize(range.length());
	return &b;
}


