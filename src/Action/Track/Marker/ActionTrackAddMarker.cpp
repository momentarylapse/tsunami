/*
 * ActionTrackAddMarker.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "ActionTrackAddMarker.h"
#include "../../../Data/TrackLayer.h"
#include "../../../Data/TrackMarker.h"

ActionTrackAddMarker::ActionTrackAddMarker(TrackLayer *l, TrackMarker *m) {
	layer = l;
	marker = m;
}

void *ActionTrackAddMarker::execute(Data *d) {
	layer->markers.add(marker.get());
	layer->notify();
	return marker.get();
}

void ActionTrackAddMarker::undo(Data *d) {
	layer->markers.pop();
	//marker->fake_death();
	layer->notify();
}

