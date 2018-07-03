/*
 * ActionTrackAddMarker.h
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#ifndef SRC_ACTION_TRACK_MARKER_ACTIONTRACKADDMARKER_H_
#define SRC_ACTION_TRACK_MARKER_ACTIONTRACKADDMARKER_H_

#include "../../Action.h"
class Track;
class TrackMarker;
class Range;

class ActionTrackAddMarker: public Action
{
public:
	ActionTrackAddMarker(Track *t, const Range &range, const string &text);
	virtual ~ActionTrackAddMarker(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	TrackMarker *marker;
	Track *track;
};

#endif /* SRC_ACTION_TRACK_MARKER_ACTIONTRACKADDMARKER_H_ */
