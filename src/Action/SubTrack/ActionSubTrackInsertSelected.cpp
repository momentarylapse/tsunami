/*
 * ActionSubTrackInsertSelected.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionSubTrackInsertSelected.h"
#include "ActionSubTrackInsert.h"

ActionSubTrackInsertSelected::ActionSubTrackInsertSelected(AudioFile *a, int level_no)
{
	foreachi(Track *t, a->track, ti)
		foreachib(SampleRef *s, t->sample, si)
			if (s->is_selected)
				AddSubAction(new ActionSubTrackInsert(a, ti, si, level_no), a);
}

ActionSubTrackInsertSelected::~ActionSubTrackInsertSelected()
{
}

