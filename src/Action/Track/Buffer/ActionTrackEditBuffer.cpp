/*
 * ActionTrackEditBuffer.cpp
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#include "ActionTrackEditBuffer.h"
#include <assert.h>

ActionTrackEditBuffer::ActionTrackEditBuffer(Track *t, int _level_no, Range _range)
{
	// prepare...
	range = _range;
	level_no = _level_no;
	track_no = get_track_index(t);

	index = -1;
	foreachi(AudioBuffer &buf, t->layers[level_no].buffers, i)
		if (buf.range().covers(range))
			index = i;
	assert(index >= 0);

	// save old data
	AudioBuffer b = t->readBuffers(level_no, range);
	box.resize(b.length);
	box.set(b, 0, 1.0f);
}



void ActionTrackEditBuffer::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	t->layers[level_no].buffers[index].invalidate_peaks(range);

	AudioBuffer b = t->readBuffers(level_no, range);
	box.swap_value(b);
}

void ActionTrackEditBuffer::redo(Data *d)
{
	undo(d);
}

void *ActionTrackEditBuffer::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	t->layers[level_no].buffers[index].invalidate_peaks(range);

	// nothing to do...
	return NULL;
}

