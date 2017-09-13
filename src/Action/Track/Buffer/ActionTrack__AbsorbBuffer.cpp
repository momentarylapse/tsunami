/*
 * ActionTrack__AbsorbBuffer.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "../../../Data/Song.h"
#include "ActionTrack__AbsorbBuffer.h"

ActionTrack__AbsorbBuffer::ActionTrack__AbsorbBuffer(Track *t, int _level_no, int _dest, int _src)
{
	track_no = get_track_index(t);
	dest = _dest;
	src = _src;
	level_no = _level_no;
}

ActionTrack__AbsorbBuffer::~ActionTrack__AbsorbBuffer()
{
}

void *ActionTrack__AbsorbBuffer::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	TrackLayer &l = t->layers[level_no];


	AudioBuffer &b_src  = l.buffers[src];
	AudioBuffer &b_dest = l.buffers[dest];
	dest_old_length = b_dest.length;
	int new_size = b_src.offset + b_src.length - b_dest.offset;
	if (new_size > b_dest.length)
		b_dest.resize(new_size);

	src_offset = l.buffers[src].offset;
	src_length = l.buffers[src].length;
	b_dest.set(b_src, b_src.offset - b_dest.offset, 1.0f);

	l.buffers.erase(src);

	return NULL;
}



void ActionTrack__AbsorbBuffer::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	TrackLayer &l = t->layers[level_no];

	//msg_todo("absorb undo...");
	AudioBuffer dummy;
	l.buffers.insert(dummy, src);
	AudioBuffer &b_src  = l.buffers[src];
	AudioBuffer &b_dest = l.buffers[dest];
	b_src.offset = src_offset;
	b_src.resize(src_length);

	b_src.set(b_dest, b_dest.offset - b_src.offset, 1.0f);
	b_dest.resize(dest_old_length);
}


