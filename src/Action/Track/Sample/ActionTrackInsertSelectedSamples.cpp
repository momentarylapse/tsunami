/*
 * ActionTrackInsertSelectedSamples.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackInsertSelectedSamples.h"

#include "../../../Data/Song.h"
#include "ActionTrackInsertSample.h"

ActionTrackInsertSelectedSamples::ActionTrackInsertSelectedSamples(Song *a, int level_no)
{
	foreachi(Track *t, a->tracks, ti)
		foreachib(SampleRef *s, t->samples, si)
			if (s->is_selected)
				addSubAction(new ActionTrackInsertSample(a, ti, si, level_no), a);
}

ActionTrackInsertSelectedSamples::~ActionTrackInsertSelectedSamples()
{
}

