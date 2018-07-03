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

class Track;
class TrackMarker;

class ActionTrackEditMarker: public Action
{
public:
	ActionTrackEditMarker(TrackMarker *m, const Range &range, const string &text);
	virtual ~ActionTrackEditMarker(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	TrackMarker *marker;
	Range range;
	string text;
};

#endif /* SRC_ACTION_TRACK_MARKER_ACTIONTRACKEDITMARKER_H_ */
