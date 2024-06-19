/*
 * ActionTrackDeleteMarker.h
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#pragma once

#include "../../Action.h"

namespace tsunami {

class TrackLayer;
class TrackMarker;

class ActionTrackDeleteMarker: public Action {
public:
	ActionTrackDeleteMarker(shared<TrackLayer> l, int index);

	string name() const override { return ":##:delete marker"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	shared<TrackMarker> marker;
	shared<TrackLayer> layer;
};

}
