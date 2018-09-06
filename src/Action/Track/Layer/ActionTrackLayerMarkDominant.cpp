/*
 * ActionTrackLayerMarkDominant.cpp
 *
 *  Created on: 04.09.2018
 *      Author: michi
 */

#include "ActionTrackLayerMarkDominant.h"
#include "ActionTrackFadeAdd.h"
#include "ActionTrackFadeDelete.h"
#include "../../../Data/Track.h"
#include "../../../Data/TrackLayer.h"

ActionTrackLayerMarkDominant::ActionTrackLayerMarkDominant(TrackLayer *_layer, const Range &_range)
{
	layer = _layer;
	range = _range;
}

void ActionTrackLayerMarkDominant::build(Data *d)
{
	Track *track = layer->track;
	int index = layer->version_number();

	int index_before = 0;
	int index_after = 0;

	for (auto &f: track->fades){
		if (f.position < range.start())
			index_before = f.target;
		if (f.position < range.end())
			index_after = f.target;
	}

	// delete fades in range
	foreachib (auto &f, track->fades, i){
		if (range.is_inside(f.position))
			addSubAction(new ActionTrackFadeDelete(track, i), d);
	}

	// add new fades
	int samples = 2000; // for now, use default value
	if (index != index_before)
		addSubAction(new ActionTrackFadeAdd(track, range.start(), samples, index), d);
	if (index != index_after)
		addSubAction(new ActionTrackFadeAdd(track, range.end(), samples, index_after), d);
}
