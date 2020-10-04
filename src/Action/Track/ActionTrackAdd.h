/*
 * ActionTrackAdd.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_ACTIONTRACKADD_H_
#define SRC_ACTION_TRACK_ACTIONTRACKADD_H_

#include "../Action.h"

class Track;

class ActionTrackAdd : public Action {
public:
	ActionTrackAdd(Track *t, int index);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	shared<Track> track;
	int index;
};

#endif /* SRC_ACTION_TRACK_ACTIONTRACKADD_H_ */
