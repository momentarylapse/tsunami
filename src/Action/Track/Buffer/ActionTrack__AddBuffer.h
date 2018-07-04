/*
 * ActionTrack__AddBuffer.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ADDBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ADDBUFFER_H_

#include "../../Action.h"
#include "../../../Data/Track.h"

// TODO: move to layer

class TrackLayer;

class ActionTrack__AddBuffer : public Action
{
public:
	ActionTrack__AddBuffer(TrackLayer *l, int index, Range r);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	TrackLayer *layer;
	int index;
	Range range;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ADDBUFFER_H_ */
