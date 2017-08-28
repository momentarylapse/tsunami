/*
 * ActionTrackMove.h
 *
 *  Created on: 28.08.2017
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_ACTIONTRACKMOVE_H_
#define SRC_ACTION_TRACK_ACTIONTRACKMOVE_H_

#include "../../Data/Song.h"
#include "../Action.h"

class Track;

class ActionTrackMove: public Action
{
public:
	ActionTrackMove(Track *track, int target);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	//Track *track;
	int origin, target;
};

#endif /* SRC_ACTION_TRACK_ACTIONTRACKMOVE_H_ */
