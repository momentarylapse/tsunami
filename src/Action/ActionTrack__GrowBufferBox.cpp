/*
 * ActionTrackGrowBufferBox.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrack__GrowBufferBox.h"
#include "../Data/Track.h"

ActionTrack__GrowBufferBox::ActionTrack__GrowBufferBox(Track *t, int _index, int _new_length)
{
	get_track_sub_index(t, track_no, sub_no);
	index = _index;
	new_length = _new_length;
}

ActionTrack__GrowBufferBox::~ActionTrack__GrowBufferBox()
{
}

void *ActionTrack__GrowBufferBox::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no, sub_no);

	BufferBox &b = t->buffer[index];
	old_length = b.num;
	b.resize(new_length);

	return NULL;
}



void ActionTrack__GrowBufferBox::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no, sub_no);

	BufferBox &b = t->buffer[index];
	b.resize(old_length);
}



void ActionTrack__GrowBufferBox::redo(Data *d)
{
	execute(d);
}


