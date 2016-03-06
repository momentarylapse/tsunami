/*
 * ActionTrackInsertSelectedSamples.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackInsertSelectedSamples.h"

#include "../../../Data/Song.h"
#include "../../../Data/SongSelection.h"
#include "ActionTrackInsertSample.h"

ActionTrackInsertSelectedSamples::ActionTrackInsertSelectedSamples(Song *a, const SongSelection &sel, int level_no)
{
	foreachi(Track *t, a->tracks, ti)
		foreachib(SampleRef *s, t->samples, si)
			if (sel.has(s))
				addSubAction(new ActionTrackInsertSample(a, ti, si, level_no), a);
}

