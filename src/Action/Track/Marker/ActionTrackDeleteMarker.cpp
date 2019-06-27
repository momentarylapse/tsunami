/*
 * ActionTrackDeleteMarker.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "ActionTrackDeleteMarker.h"
#include "../../../Data/Track.h"
#include "../../../Data/TrackMarker.h"
#include <assert.h>

ActionTrackDeleteMarker::ActionTrackDeleteMarker(Track *t, int _index) {
	track = t;
	index = _index;
	marker = nullptr;
}

ActionTrackDeleteMarker::~ActionTrackDeleteMarker() {
	if (marker)
		delete marker;
}

void *ActionTrackDeleteMarker::execute(Data *d) {
	assert(index >= 0);
	assert(index < track->markers.num);

	marker = track->markers[index];
	//marker->fake_death();
	track->markers.erase(index);
	marker = nullptr;

	track->notify();
	return nullptr;
}

void ActionTrackDeleteMarker::undo(Data *d) {
	track->markers.insert(marker, index);
	track->notify();
	marker = nullptr;
}

