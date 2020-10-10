/*
 * ActionTrackEditBuffer.h
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACKEDITBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACKEDITBUFFER_H_

#include "../../Action.h"
#include "../../../Data/Audio/AudioBuffer.h"

class TrackLayer;

class ActionTrackEditBuffer : public Action {
public:
	ActionTrackEditBuffer(TrackLayer *l, const Range &_range);

	void *execute(Data *d) override;
	void undo(Data *d) override;
	void redo(Data *d) override;

private:
	TrackLayer *layer;
	Range range;
	AudioBuffer box;
	int index;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACKEDITBUFFER_H_ */
