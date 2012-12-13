/*
 * ActionSubTrackInsertSelected.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionSubTrackInsertSelected.h"
#include "ActionSubTrackInsert.h"

ActionSubTrackInsertSelected::ActionSubTrackInsertSelected(AudioFile *a)
{
	foreachi(Track *t, a->track, ti)
		foreachib(Track *s, t->sub, si)
			if (s->is_selected)
				AddSubAction(new ActionSubTrackInsert(a, ti, si, a->cur_level), a);
}

ActionSubTrackInsertSelected::~ActionSubTrackInsertSelected()
{
}

