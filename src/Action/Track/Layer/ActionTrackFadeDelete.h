/*
 * ActionTrackFadeDelete.h
 *
 *  Created on: 05.09.2018
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_LAYER_ACTIONTRACKFADEDELETE_H_
#define SRC_ACTION_TRACK_LAYER_ACTIONTRACKFADEDELETE_H_

#include "../../Action.h"

class Track;

class ActionTrackFadeDelete : public Action
{
public:
	ActionTrackFadeDelete(Track *t, int index);

	void *execute(Data *d) override;
	void undo(Data *d) override;

	Track *track;
	int position, samples, target;
	int index;
};

#endif /* SRC_ACTION_TRACK_LAYER_ACTIONTRACKFADEDELETE_H_ */
