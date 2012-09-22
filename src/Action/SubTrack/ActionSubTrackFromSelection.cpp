/*
 * ActionSubTrackFromSelection.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionSubTrackFromSelection.h"
#include "../Track/ActionTrackAddEmptySubTrack.h"
#include "../Track/ActionTrackEditBuffer.h"
#include "../Track/ActionTrack__DeleteBufferBox.h"

ActionSubTrackFromSelection::ActionSubTrackFromSelection(AudioFile *a)
{
	foreachi(Track &t, a->track, ti)
		if (t.is_selected)
			CreateSubsFromTrack(a, &t, t.level[a->cur_level], ti);
}

ActionSubTrackFromSelection::~ActionSubTrackFromSelection()
{
}


void ActionSubTrackFromSelection::CreateSubsFromTrack(AudioFile *a, Track *t, TrackLevel &l, int track_no)
{
	foreachib(BufferBox &b, l.buffer, bi)
		if (a->selection.covers(b.range())){
			Track *s = (Track*)AddSubAction(new ActionTrackAddEmptySubTrack(track_no, b.range(), "-new-"), a);

			Action *action = new ActionTrackEditBuffer(s, 0, Range(0, b.num));
			s->level[0].buffer[0].set(b, 0, 1.0f);
			AddSubAction(action, a);

			AddSubAction(new ActionTrack__DeleteBufferBox(t, a->cur_level, bi), a);
		}
}
