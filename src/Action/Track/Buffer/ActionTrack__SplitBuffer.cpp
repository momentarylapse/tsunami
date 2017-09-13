/*
 * ActionTrack__SplitBuffer.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include <assert.h>

#include "ActionTrack__SplitBuffer.h"

ActionTrack__SplitBuffer::ActionTrack__SplitBuffer(Track *t, int _level_no, int _index, int _offset)
{
	track_no = get_track_index(t);
	index = _index;
	offset = _offset;
	level_no = _level_no;
}



void ActionTrack__SplitBuffer::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	AudioBuffer &b = t->layers[level_no].buffers[index];
	AudioBuffer &b2 = t->layers[level_no].buffers[index + 1];

	// transfer data
	b.resize(b.length + b2.length);
	b.set(b2, offset, 1.0f);

	// delete
	t->layers[level_no].buffers.erase(index + 1);
}



void *ActionTrack__SplitBuffer::execute(Data *d)
{
	//msg_write(format("cut %d   at %d", index, offset));
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	TrackLayer &l = t->layers[level_no];

	assert(offset > 0);
	assert(offset < (l.buffers[index].length - 1));

	// create new
	AudioBuffer dummy;
	l.buffers.insert(dummy, index + 1);

	AudioBuffer &b = l.buffers[index];
	AudioBuffer &b2 = l.buffers[index + 1];

	// new position
	b2.offset = b.offset + offset;

	// transfer data
	b2.resize(b.length - offset);
	b2.set(b, -offset, 1.0f);
	b.resize(offset);
	return &b;
}


