/*
 * ActionTrack__AddBufferBox.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrack__AddBufferBox.h"
#include "../../../Data/Song.h"
#include <assert.h>

ActionTrack__AddBufferBox::ActionTrack__AddBufferBox(Track *t, int _level_no, int _index, Range r)
{
	track_no = get_track_index(t);
	index = _index;
	range = r;
	level_no = _level_no;
}

void ActionTrack__AddBufferBox::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	// should be zeroes at this point...
	t->levels[level_no].buffers.erase(index);
}



void *ActionTrack__AddBufferBox::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	assert(t and "AddBufferBox.execute");

	BufferBox dummy;
	t->levels[level_no].buffers.insert(dummy, index);

	// reserve memory
	BufferBox &b = t->levels[level_no].buffers[index];
	b.offset = range.start();
	b.resize(range.length);
	return &b;
}


