/*
 * ActionTrackAddMarker.h
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

struct TrackLayer;
struct TrackMarker;
struct Range;

class ActionTrackAddMarker: public history::Action {
public:
	ActionTrackAddMarker(TrackLayer *l, const shared<TrackMarker> marker);

	string name() const override { return ":##:add marker"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	shared<TrackMarker> marker;
	TrackLayer *layer;
};

}
