/*
 * ActionTrackGrowBufferBox.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrack__GrowBufferBox.h"
#include "../../../Data/Track.h"

ActionTrack__GrowBufferBox::ActionTrack__GrowBufferBox(Track *t, int _level_no, int _index, int _new_length)
{
	track_no = get_track_index(t);
	level_no = _level_no;
	index = _index;
	new_length = _new_length;
}

ActionTrack__GrowBufferBox::~ActionTrack__GrowBufferBox()
{
}

void *ActionTrack__GrowBufferBox::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	BufferBox &b = t->levels[level_no].buffers[index];
	old_length = b.num;
	b.resize(new_length);

	return NULL;
}



void ActionTrack__GrowBufferBox::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	BufferBox &b = t->levels[level_no].buffers[index];
	b.resize(old_length);
}


