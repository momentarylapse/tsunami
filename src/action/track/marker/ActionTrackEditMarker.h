/*
 * ActionTrackEditMarker.h
 *
 *  Created on: 03.10.2017
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>
#include "../../../data/Range.h"

namespace tsunami {

class TrackLayer;
class TrackMarker;

class ActionTrackEditMarker: public history::Action {
public:
	ActionTrackEditMarker(const TrackLayer *l, TrackMarker *m, const Range &range, const string &text);

	string name() const override { return ":##:edit marker"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	const TrackLayer *layer;
	TrackMarker *marker;
	Range range;
	string text;
};

}
