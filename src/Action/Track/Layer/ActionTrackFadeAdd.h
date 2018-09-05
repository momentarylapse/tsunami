/*
 * ActionTrackFadeAdd.h
 *
 *  Created on: 05.09.2018
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_LAYER_ACTIONTRACKFADEADD_H_
#define SRC_ACTION_TRACK_LAYER_ACTIONTRACKFADEADD_H_

#include "../../Action.h"

class Track;

class ActionTrackFadeAdd : public Action
{
public:
	ActionTrackFadeAdd(Track *t, int position, int samples, int target);

	void *execute(Data *d) override;
	void undo(Data *d) override;

	Track *track;
	int position, samples, target;
	int index;
};

#endif /* SRC_ACTION_TRACK_LAYER_ACTIONTRACKFADEADD_H_ */
