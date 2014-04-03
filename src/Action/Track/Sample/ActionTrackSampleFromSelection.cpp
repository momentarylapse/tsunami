/*
 * ActionTrackSampleFromSelection.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackSampleFromSelection.h"
#include "ActionTrackPasteAsSample.h"
#include "../Buffer/ActionTrackEditBuffer.h"
#include "../Buffer/ActionTrack__DeleteBufferBox.h"

ActionTrackSampleFromSelection::ActionTrackSampleFromSelection(AudioFile *a, const Range &r, int level_no)
{
	foreach(Track *t, a->track)
		if (t->is_selected)
			CreateSubsFromTrack(t, r, level_no);
}

ActionTrackSampleFromSelection::~ActionTrackSampleFromSelection()
{
}


void ActionTrackSampleFromSelection::CreateSubsFromTrack(Track *t, const Range &r, int level_no)
{
	AudioFile *a = t->root;
	TrackLevel &l = t->level[level_no];
	foreachib(BufferBox &b, l.buffer, bi)
		if (r.covers(b.range())){
			AddSubAction(new ActionTrackPasteAsSample(t, b.offset, &b), a);

			AddSubAction(new ActionTrack__DeleteBufferBox(t, level_no, bi), a);
		}
}
