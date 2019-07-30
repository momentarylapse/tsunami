/*
 * ActionTrackDeleteMarker.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "ActionTrackDeleteMarker.h"
#include "../../../Data/TrackLayer.h"
#include "../../../Data/TrackMarker.h"
#include <assert.h>

ActionTrackDeleteMarker::ActionTrackDeleteMarker(TrackLayer *l, int _index) {
	layer = l;
	index = _index;
	marker = nullptr;
}

ActionTrackDeleteMarker::~ActionTrackDeleteMarker() {
	if (marker)
		delete marker;
}

void *ActionTrackDeleteMarker::execute(Data *d) {
	assert(index >= 0);
	assert(index < layer->markers.num);

	marker = layer->markers[index];
	//marker->fake_death();
	layer->markers.erase(index);
	marker = nullptr;

	layer->notify();
	return nullptr;
}

void ActionTrackDeleteMarker::undo(Data *d) {
	layer->markers.insert(marker, index);
	layer->notify();
	marker = nullptr;
}

