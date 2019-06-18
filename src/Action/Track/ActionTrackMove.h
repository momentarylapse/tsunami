/*
 * ActionTrackMove.h
 *
 *  Created on: 28.08.2017
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_ACTIONTRACKMOVE_H_
#define SRC_ACTION_TRACK_ACTIONTRACKMOVE_H_

#include "../Action.h"

class Track;

class ActionTrackMove: public Action {
public:
	ActionTrackMove(Track *track, int target);

	void *execute(Data *d) override;
	void undo(Data *d) override;

	int origin, target;
};

#endif /* SRC_ACTION_TRACK_ACTIONTRACKMOVE_H_ */
