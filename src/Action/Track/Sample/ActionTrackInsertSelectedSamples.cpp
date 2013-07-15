/*
 * ActionTrackInsertSelectedSamples.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackInsertSelectedSamples.h"
#include "ActionTrackInsertSample.h"
#include "../../../Data/AudioFile.h"

ActionTrackInsertSelectedSamples::ActionTrackInsertSelectedSamples(AudioFile *a, int level_no)
{
	foreachi(Track *t, a->track, ti)
		foreachib(SampleRef *s, t->sample, si)
			if (s->is_selected)
				AddSubAction(new ActionTrackInsertSample(a, ti, si, level_no), a);
}

ActionTrackInsertSelectedSamples::~ActionTrackInsertSelectedSamples()
{
}

