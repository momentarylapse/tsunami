/*
 * ActionTrackMoveBuffer.h
 *
 *  Created on: Sep 24, 2020
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACKMOVEBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACKMOVEBUFFER_H_

#include "../../Action.h"

class TrackLayer;

class ActionTrackMoveBuffer : public Action {
public:
	ActionTrackMoveBuffer(TrackLayer *l, int index, int shift);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	TrackLayer *layer;
	int index;
	int shift;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACKMOVEBUFFER_H_ */
