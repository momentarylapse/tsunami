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
#include "../../../Data/SongSelection.h"

ActionTrackSampleFromSelection::ActionTrackSampleFromSelection(Song *a, const SongSelection &sel, int level_no)
{
	for (Track *t : a->tracks)
		if (sel.has(t))
			CreateSubsFromTrack(t, sel, level_no);
}


void ActionTrackSampleFromSelection::CreateSubsFromTrack(Track *t, const SongSelection &sel, int level_no)
{
	Song *a = t->song;
	TrackLevel &l = t->levels[level_no];
	foreachib(BufferBox &b, l.buffers, bi)
		if (sel.range.covers(b.range())){
			addSubAction(new ActionTrackPasteAsSample(t, b.offset, &b), a);

			addSubAction(new ActionTrack__DeleteBufferBox(t, level_no, bi), a);
		}
}
