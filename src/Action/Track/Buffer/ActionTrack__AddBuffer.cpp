/*
 * ActionTrack__AddBuffer.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "../../../Data/Song.h"
#include <assert.h>
#include "ActionTrack__AddBuffer.h"

ActionTrack__AddBuffer::ActionTrack__AddBuffer(Track *t, int _level_no, int _index, Range r)
{
	track_no = get_track_index(t);
	index = _index;
	range = r;
	level_no = _level_no;
}

void ActionTrack__AddBuffer::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	// should be zeroes at this point...
	t->layers[level_no].buffers.erase(index);
}



void *ActionTrack__AddBuffer::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	assert(t and "AddBufferBox.execute");

	AudioBuffer dummy;
	t->layers[level_no].buffers.insert(dummy, index);

	// reserve memory
	AudioBuffer &b = t->layers[level_no].buffers[index];
	b.offset = range.start();
	b.resize(range.length);
	return &b;
}


