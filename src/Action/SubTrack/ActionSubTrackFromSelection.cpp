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

ActionSubTrackFromSelection::ActionSubTrackFromSelection(AudioFile *a, int level_no)
{
	foreachi(Track *t, a->track, ti)
		if (t->is_selected)
			CreateSubsFromTrack(a, t, ti, level_no);
}

ActionSubTrackFromSelection::~ActionSubTrackFromSelection()
{
}


void ActionSubTrackFromSelection::CreateSubsFromTrack(AudioFile *a, Track *t, int track_no, int level_no)
{
	TrackLevel &l = t->level[level_no];
	foreachib(BufferBox &b, l.buffer, bi)
		if (a->selection.covers(b.range())){
			Track *s = (Track*)AddSubAction(new ActionTrackAddEmptySubTrack(track_no, b.range(), "-new-"), a);

			Action *action = new ActionTrackEditBuffer(s, 0, Range(0, b.num));
			s->level[0].buffer[0].set(b, 0, 1.0f);
			AddSubAction(action, a);

			AddSubAction(new ActionTrack__DeleteBufferBox(t, level_no, bi), a);
		}
}
