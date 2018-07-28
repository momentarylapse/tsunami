/*
 * ActionTrackInsertSelectedSamples.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackInsertSelectedSamples.h"

#include "../../../Data/Track.h"
#include "../../../Data/Song.h"
#include "../../../Data/SampleRef.h"
#include "../../../Data/SongSelection.h"
#include "ActionTrackInsertSample.h"

ActionTrackInsertSelectedSamples::ActionTrackInsertSelectedSamples(const SongSelection &_sel) :
	sel(_sel)
{
}

void ActionTrackInsertSelectedSamples::build(Data *d)
{
	auto *s = dynamic_cast<Song*>(d);

	for (TrackLayer *l: s->layers())
		foreachib(SampleRef *ss, l->samples, si)
			if (sel.has(ss))
				addSubAction(new ActionTrackInsertSample(l, si), d);
}

