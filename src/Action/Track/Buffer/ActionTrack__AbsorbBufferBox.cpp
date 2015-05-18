/*
 * ActionTrack__AbsorbBufferBox.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrack__AbsorbBufferBox.h"
#include "../../../Data/AudioFile.h"

ActionTrack__AbsorbBufferBox::ActionTrack__AbsorbBufferBox(Track *t, int _level_no, int _dest, int _src)
{
	track_no = get_track_index(t);
	dest = _dest;
	src = _src;
	level_no = _level_no;
}

ActionTrack__AbsorbBufferBox::~ActionTrack__AbsorbBufferBox()
{
}

void *ActionTrack__AbsorbBufferBox::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);
	TrackLevel &l = t->levels[level_no];


	BufferBox &b_src  = l.buffers[src];
	BufferBox &b_dest = l.buffers[dest];
	dest_old_length = b_dest.num;
	int new_size = b_src.offset + b_src.num - b_dest.offset;
	if (new_size > b_dest.num)
		b_dest.resize(new_size);

	src_offset = l.buffers[src].offset;
	src_length = l.buffers[src].num;
	b_dest.set(b_src, b_src.offset - b_dest.offset, 1.0f);

	l.buffers.erase(src);

	return NULL;
}



void ActionTrack__AbsorbBufferBox::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);
	TrackLevel &l = t->levels[level_no];

	//msg_todo("absorb undo...");
	BufferBox dummy;
	l.buffers.insert(dummy, src);
	BufferBox &b_src  = l.buffers[src];
	BufferBox &b_dest = l.buffers[dest];
	b_src.offset = src_offset;
	b_src.resize(src_length);

	b_src.set(b_dest, b_dest.offset - b_src.offset, 1.0f);
	b_dest.resize(dest_old_length);
}


