/*
 * ActionTrackAddMarker.h
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#ifndef SRC_ACTION_TRACK_MARKER_ACTIONTRACKADDMARKER_H_
#define SRC_ACTION_TRACK_MARKER_ACTIONTRACKADDMARKER_H_

#include "../../Action.h"

class TrackLayer;
class TrackMarker;
class Range;

class ActionTrackAddMarker: public Action {
public:
	ActionTrackAddMarker(TrackLayer *l, const TrackMarker *marker);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	shared<TrackMarker> marker;
	TrackLayer *layer;
};

#endif /* SRC_ACTION_TRACK_MARKER_ACTIONTRACKADDMARKER_H_ */
