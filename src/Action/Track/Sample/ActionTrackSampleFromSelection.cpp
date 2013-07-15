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

ActionTrackSampleFromSelection::ActionTrackSampleFromSelection(AudioFile *a, int level_no)
{
	foreachi(Track *t, a->track, ti)
		if (t->is_selected)
			CreateSubsFromTrack(a, t, ti, level_no);
}

ActionTrackSampleFromSelection::~ActionTrackSampleFromSelection()
{
}


void ActionTrackSampleFromSelection::CreateSubsFromTrack(AudioFile *a, Track *t, int track_no, int level_no)
{
	TrackLevel &l = t->level[level_no];
	foreachib(BufferBox &b, l.buffer, bi)
		if (a->selection.covers(b.range())){
			AddSubAction(new ActionTrackPasteAsSample(a, track_no, b.offset, &b), a);

			AddSubAction(new ActionTrack__DeleteBufferBox(t, level_no, bi), a);
		}
}
