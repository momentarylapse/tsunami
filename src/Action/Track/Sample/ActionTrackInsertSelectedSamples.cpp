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

ActionTrackInsertSelectedSamples::ActionTrackInsertSelectedSamples(const SongSelection &_sel, int _layer_no) :
	sel(_sel)
{
	layer_no = _layer_no;
}

void ActionTrackInsertSelectedSamples::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	foreachi(Track *t, s->tracks, ti)
		foreachib(SampleRef *ss, t->samples, si)
			if (sel.has(ss))
				addSubAction(new ActionTrackInsertSample(ti, si, layer_no), d);
}

