/*
 * ActionTrack__SplitBuffer.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__SPLITBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__SPLITBUFFER_H_

#include "../../Action.h"
#include "../../../Data/Track.h"

class ActionTrack__SplitBuffer : public Action
{
public:
	ActionTrack__SplitBuffer(TrackLayer *l, int _index, int _offset);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	TrackLayer *layer;
	int index;
	int offset;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__SPLITBUFFER_H_ */
