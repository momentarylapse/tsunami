/*
 * ActionTrackAddMarker.h
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#pragma once

#include "../../Action.h"

class TrackLayer;
class TrackMarker;
class Range;

class ActionTrackAddMarker: public Action {
public:
	ActionTrackAddMarker(TrackLayer *l, const TrackMarker *marker);

	string name() const override { return ":##:add marker"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	shared<TrackMarker> marker;
	TrackLayer *layer;
};
