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
	foreachi(a->track, t, ti)
		foreachbi(t->sub, s, si)
			if (s->is_selected)
				AddSubAction(new ActionSubTrackInsert(a, ti, si, a->cur_level), a);
}

ActionSubTrackInsertSelected::~ActionSubTrackInsertSelected()
{
}

