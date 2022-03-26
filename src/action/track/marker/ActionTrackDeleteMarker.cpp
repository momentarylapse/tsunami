/*
 * ActionTrackDeleteMarker.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "ActionTrackDeleteMarker.h"
#include "../../../data/TrackLayer.h"
#include "../../../data/TrackMarker.h"
#include <assert.h>

ActionTrackDeleteMarker::ActionTrackDeleteMarker(shared<TrackLayer> l, int _index) {
	layer = l;
	index = _index;
}

void *ActionTrackDeleteMarker::execute(Data *d) {
	assert(index >= 0);
	assert(index < layer->markers.num);

	marker = layer->markers[index];
	//marker->fake_death();
	layer->markers.erase(index);

	layer->notify();
	return nullptr;
}

void ActionTrackDeleteMarker::undo(Data *d) {
	layer->markers.insert(marker.get(), index);
	layer->notify();
}

