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

class ActionTrackAddMarker: public Action
{
public:
	ActionTrackAddMarker(Track *t, int pos, const string &text);
	virtual ~ActionTrackAddMarker(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	TrackMarker *marker;
	int track_no;
};

#endif /* SRC_ACTION_TRACK_MARKER_ACTIONTRACKADDMARKER_H_ */
