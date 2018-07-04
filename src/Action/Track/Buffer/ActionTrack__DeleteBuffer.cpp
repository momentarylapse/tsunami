/*
 * ActionTrack__DeleteBuffer.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include <assert.h>
#include "ActionTrack__DeleteBuffer.h"

ActionTrack__DeleteBuffer::ActionTrack__DeleteBuffer(TrackLayer *l, int _index)
{
	layer = l;
	index = _index;
}



void ActionTrack__DeleteBuffer::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	// restore
	layer->buffers.insert(buf, index);

	// clean up
	buf.clear();
}



void *ActionTrack__DeleteBuffer::execute(Data *d)
{
	//msg_write("delete " + i2s(index));
	Song *a = dynamic_cast<Song*>(d);

	AudioBuffer &b = layer->buffers[index];

	assert(index >= 0 and index < layer->buffers.num);

	// save data
	buf = b;

	// delete
	layer->buffers.erase(index);
	return NULL;
}


