/*
 * ActionTrackEditMarker.cpp
 *
 *  Created on: 03.10.2017
 *      Author: michi
 */

#include "ActionTrackEditMarker.h"
#include "../../../data/TrackLayer.h"
#include "../../../data/TrackMarker.h"
#include <assert.h>

ActionTrackEditMarker::ActionTrackEditMarker(const TrackLayer *l, TrackMarker *m, const Range &_range, const string &_text) {
	layer = l;
	marker = m;
	range = _range;
	text = _text;
}

void *ActionTrackEditMarker::execute(Data *d) {
	std::swap(marker->text, text);
	std::swap(marker->range, range);

	layer->notify();
	return nullptr;
}

void ActionTrackEditMarker::undo(Data *d) {
	execute(d);
}

