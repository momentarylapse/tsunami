/*
 * ActionTrackDeleteMarker.h
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

struct TrackLayer;
struct TrackMarker;

class ActionTrackDeleteMarker: public history::Action {
public:
	ActionTrackDeleteMarker(shared<TrackLayer> l, int index);

	string name() const override { return ":##:delete marker"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int index;
	shared<TrackMarker> marker;
	shared<TrackLayer> layer;
};

}
