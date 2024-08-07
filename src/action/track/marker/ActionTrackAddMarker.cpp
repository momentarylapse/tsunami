/*
 * ActionTrackAddMarker.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "ActionTrackAddMarker.h"
#include "../../../data/TrackLayer.h"
#include "../../../data/TrackMarker.h"

namespace tsunami {

ActionTrackAddMarker::ActionTrackAddMarker(TrackLayer *l, const shared<TrackMarker> m) {
	layer = l;
	marker = const_cast<TrackMarker*>(m.get());
}

void *ActionTrackAddMarker::execute(Data *d) {
	layer->markers.add(marker.get());
	layer->out_changed.notify();
	return marker.get();
}

void ActionTrackAddMarker::undo(Data *d) {
	layer->markers.pop();
	//marker->fake_death();
	layer->out_changed.notify();
}

}

