/*
 * ActionTrack__AddBufferBox.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrack__AddBufferBox.h"
#include "../Data/AudioFile.h"

ActionTrack__AddBufferBox::ActionTrack__AddBufferBox(int _track_no, int _index, int _pos, int _length)
{
	track_no = _track_no;
	index = _index;
	pos = _pos;
	length = _length;
}

ActionTrack__AddBufferBox::~ActionTrack__AddBufferBox()
{
}

void ActionTrack__AddBufferBox::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track &t = a->track[track_no];

	// should be zeroes at this point...
	t.buffer.erase(index);
}



void ActionTrack__AddBufferBox::redo(Data *d)
{
	execute(d);
}



void *ActionTrack__AddBufferBox::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track &t = a->track[track_no];

	BufferBox dummy;
	t.buffer.insert(dummy, index);

	// reserve memory
	BufferBox &b = t.buffer[index];
	b.offset = pos;
	b.resize(length);
	return &b;
}


