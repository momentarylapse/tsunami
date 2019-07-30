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

ActionTrackAddMarker::~ActionTrackAddMarker() {
	if (marker)
		delete marker;
}

void *ActionTrackAddMarker::execute(Data *d) {
	layer->markers.add(marker);
	layer->notify();
	auto *m = marker;
	marker = nullptr;
	return m;
}

void ActionTrackAddMarker::undo(Data *d) {
	marker = layer->markers.pop();
	//marker->fake_death();
	layer->notify();
}

