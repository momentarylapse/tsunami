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

ActionTrackSampleFromSelection::ActionTrackSampleFromSelection(const SongSelection &_sel, int _level_no) :
	sel(_sel)
{
	level_no = _level_no;
}

void ActionTrackSampleFromSelection::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	for (Track *t: s->tracks)
		if (sel.has(t))
			CreateSubsFromTrack(t, sel, level_no);
}


void ActionTrackSampleFromSelection::CreateSubsFromTrack(Track *t, const SongSelection &sel, int level_no)
{
	TrackLevel &l = t->levels[level_no];
	foreachib(BufferBox &b, l.buffers, bi)
		if (sel.range.covers(b.range())){
			addSubAction(new ActionTrackPasteAsSample(t, b.offset, b), t->song);

			addSubAction(new ActionTrack__DeleteBufferBox(t, level_no, bi), t->song);
		}
}
