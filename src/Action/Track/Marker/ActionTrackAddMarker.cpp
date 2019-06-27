/*
 * ActionTrackAddMarker.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "ActionTrackAddMarker.h"
#include "../../../Data/Track.h"
#include "../../../Data/TrackMarker.h"

ActionTrackAddMarker::ActionTrackAddMarker(Track *t, TrackMarker *m) {
	track = t;
	marker = m;
}

ActionTrackAddMarker::~ActionTrackAddMarker() {
	if (marker)
		delete marker;
}

void *ActionTrackAddMarker::execute(Data *d) {
	track->markers.add(marker);
	track->notify();
	auto *m = marker;
	marker = nullptr;
	return m;
}

void ActionTrackAddMarker::undo(Data *d) {
	marker = track->markers.pop();
	//marker->fake_death();
	track->notify();
}

