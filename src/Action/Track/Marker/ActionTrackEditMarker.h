/*
 * ActionTrackEditMarker.h
 *
 *  Created on: 03.10.2017
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_MARKER_ACTIONTRACKEDITMARKER_H_
#define SRC_ACTION_TRACK_MARKER_ACTIONTRACKEDITMARKER_H_

#include "../../Action.h"
#include "../../../Data/Range.h"

class TrackLayer;
class TrackMarker;

class ActionTrackEditMarker: public Action {
public:
	ActionTrackEditMarker(const TrackLayer *l, TrackMarker *m, const Range &range, const string &text);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	const TrackLayer *layer;
	TrackMarker *marker;
	Range range;
	string text;
};

#endif /* SRC_ACTION_TRACK_MARKER_ACTIONTRACKEDITMARKER_H_ */
