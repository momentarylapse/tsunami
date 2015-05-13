/*
 * ActionTrackMoveMarker.h
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#ifndef SRC_ACTION_TRACK_MARKER_ACTIONTRACKMOVEMARKER_H_
#define SRC_ACTION_TRACK_MARKER_ACTIONTRACKMOVEMARKER_H_

#include "../../Action.h"
class Track;

class ActionTrackMoveMarker: public Action
{
public:
	ActionTrackMoveMarker(Track *t, int index, int pos);
	virtual ~ActionTrackMoveMarker(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index, pos;
	string text;
	int track_no;
};

#endif /* SRC_ACTION_TRACK_MARKER_ACTIONTRACKMOVEMARKER_H_ */
