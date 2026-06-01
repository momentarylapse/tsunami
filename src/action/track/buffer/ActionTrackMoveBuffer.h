/*
 * ActionTrackMoveBuffer.h
 *
 *  Created on: Sep 24, 2020
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACKMOVEBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACKMOVEBUFFER_H_

#include <lib/history/Action.h>

namespace tsunami {

class TrackLayer;

class ActionTrackMoveBuffer : public history::Action {
public:
	ActionTrackMoveBuffer(TrackLayer *l, int index, int shift);

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	TrackLayer *layer;
	int index;
	int shift;
};

}

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACKMOVEBUFFER_H_ */
