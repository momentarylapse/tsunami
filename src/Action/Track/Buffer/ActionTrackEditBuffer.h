/*
 * ActionTrackEditBuffer.h
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACKEDITBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACKEDITBUFFER_H_

#include "../../Action.h"
#include "../../../Data/Track.h"

class ActionTrackEditBuffer : public Action
{
public:
	ActionTrackEditBuffer(TrackLayer *l, Range _range);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
	virtual void redo(Data *d);

private:
	TrackLayer *layer;
	Range range;
	AudioBuffer box;
	int index;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACKEDITBUFFER_H_ */
