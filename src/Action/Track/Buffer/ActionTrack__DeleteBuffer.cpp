/*
 * ActionTrack__DeleteBuffer.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include <assert.h>
#include "ActionTrack__DeleteBuffer.h"

ActionTrack__DeleteBuffer::ActionTrack__DeleteBuffer(Track *t, int _level_no, int _index)
{
	track_no = get_track_index(t);
	index = _index;
	level_no = _level_no;
}



void ActionTrack__DeleteBuffer::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	// restore
	t->layers[level_no].buffers.insert(buf, index);

	// clean up
	buf.clear();
}



void *ActionTrack__DeleteBuffer::execute(Data *d)
{
	//msg_write("delete " + i2s(index));
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	assert(level_no >= 0);
	assert(level_no < t->layers.num);
	AudioBuffer &b = t->layers[level_no].buffers[index];

	assert(index >= 0 and index < t->layers[level_no].buffers.num);

	// save data
	buf = b;

	// delete
	t->layers[level_no].buffers.erase(index);
	return NULL;
}


