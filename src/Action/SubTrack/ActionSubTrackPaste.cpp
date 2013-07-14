/*
 * ActionSubTrackPaste.cpp
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#include "ActionSubTrackPaste.h"
#include <assert.h>

ActionSubTrackPaste::ActionSubTrackPaste(int track_no, int pos, BufferBox *buf)
{
	/*sub = new Track;
	sub->parent = track_no;
	sub->pos = pos;
	sub->length = buf->num;
	sub->name = "-paste-";
	sub->level.resize(1);
	sub->level[0].buffer.resize(1);
	sub->level[0].buffer[0] = *buf;
	sub->level[0].buffer[0].offset = 0;*/
}

ActionSubTrackPaste::~ActionSubTrackPaste()
{
}

void *ActionSubTrackPaste::execute(Data *d)
{
	/*AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(sub->parent >= 0);
	assert(sub->parent < a->track.num);
	Track *t = a->track[sub->parent];

	sub->root = a;
	sub->is_selected = true;
	t->sub.add(sub);*/

	return NULL;
}

void ActionSubTrackPaste::undo(Data *d)
{
	/*AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->track[sub->parent];
	t->sub.pop();*/
}

