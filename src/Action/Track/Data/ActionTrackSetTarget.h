/*
 * ActionTrackSetTarget.h
 *
 *  Created on: 18.06.2019
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_DATA_ACTIONTRACKSETTARGET_H_
#define SRC_ACTION_TRACK_DATA_ACTIONTRACKSETTARGET_H_

#include "../../Action.h"

class Track;

class ActionTrackSetTarget : public Action {
public:
	ActionTrackSetTarget(Track *t, Track *target);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Track *track;
	Track *target;
};

#endif /* SRC_ACTION_TRACK_DATA_ACTIONTRACKSETTARGET_H_ */
