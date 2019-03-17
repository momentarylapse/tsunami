/*
 * ActionTrackDeleteMarker.h
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#ifndef SRC_ACTION_TRACK_MARKER_ACTIONTRACKDELETEMARKER_H_
#define SRC_ACTION_TRACK_MARKER_ACTIONTRACKDELETEMARKER_H_

#include "../../Action.h"
class Track;
class TrackMarker;

class ActionTrackDeleteMarker: public Action
{
public:
	ActionTrackDeleteMarker(Track *t, int index);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	TrackMarker *marker;
	Track *track;
};

#endif /* SRC_ACTION_TRACK_MARKER_ACTIONTRACKDELETEMARKER_H_ */
