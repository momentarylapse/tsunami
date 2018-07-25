/*
 * ActionTrackInsertSelectedSamples.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackInsertSelectedSamples.h"

#include "../../../Data/Track.h"
#include "../../../Data/SampleRef.h"
#include "../../../Data/SongSelection.h"
#include "ActionTrackInsertSample.h"

ActionTrackInsertSelectedSamples::ActionTrackInsertSelectedSamples(const SongSelection &_sel, TrackLayer *l) :
	sel(_sel)
{
	layer = l;
}

void ActionTrackInsertSelectedSamples::build(Data *d)
{
	foreachib(SampleRef *ss, layer->samples, si)
		if (sel.has(ss))
			addSubAction(new ActionTrackInsertSample(layer, si), d);
}

