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

ActionTrackSampleFromSelection::ActionTrackSampleFromSelection(Song *a, const Range &r, int level_no)
{
	foreach(Track *t, a->tracks)
		if (t->is_selected)
			CreateSubsFromTrack(t, r, level_no);
}

ActionTrackSampleFromSelection::~ActionTrackSampleFromSelection()
{
}


void ActionTrackSampleFromSelection::CreateSubsFromTrack(Track *t, const Range &r, int level_no)
{
	Song *a = t->song;
	TrackLevel &l = t->levels[level_no];
	foreachib(BufferBox &b, l.buffers, bi)
		if (r.covers(b.range())){
			addSubAction(new ActionTrackPasteAsSample(t, b.offset, &b), a);

			addSubAction(new ActionTrack__DeleteBufferBox(t, level_no, bi), a);
		}
}
