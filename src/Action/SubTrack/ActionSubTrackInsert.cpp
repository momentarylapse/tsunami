/*
 * ActionSubTrackInsert.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionSubTrackInsert.h"
#include "ActionSubTrackDelete.h"
#include "../Track/Buffer/ActionTrackCreateBuffers.h"
#include "../Track/Buffer/ActionTrackEditBuffer.h"

ActionSubTrackInsert::ActionSubTrackInsert(AudioFile *a, int track_no, int index, int level_no)
{
	Track *sub = a->track[track_no]->sub[index];

	// get target buffer
	Range r = sub->GetRange();
	AddSubAction(new ActionTrackCreateBuffers(a->track[track_no], level_no, r), a);
	BufferBox buf = a->track[track_no]->ReadBuffers(level_no, r);

	// insert sub (ignore muted)
	ActionTrackEditBuffer *action = new ActionTrackEditBuffer(a->track[track_no], level_no, r);
	buf.set(sub->level[0].buffer[0], 0, sub->volume);
	AddSubAction(action, a);

	// delete sub
	AddSubAction(new ActionSubTrackDelete(track_no, index), a);
}

ActionSubTrackInsert::~ActionSubTrackInsert()
{
}

