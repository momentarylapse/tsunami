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

class ActionTrackDeleteMarker: public Action
{
public:
	ActionTrackDeleteMarker(Track *t, int index);
	virtual ~ActionTrackDeleteMarker(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index;
	int pos;
	string text;
	int track_no;
};

#endif /* SRC_ACTION_TRACK_MARKER_ACTIONTRACKDELETEMARKER_H_ */
