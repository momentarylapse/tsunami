/*
 * ActionTrackGrowBuffer.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__GROWBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__GROWBUFFER_H_

#include "../../Action.h"

class TrackLayer;

class ActionTrack__GrowBuffer : public Action {
public:
	ActionTrack__GrowBuffer(TrackLayer *l, int _index, int _new_length);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	TrackLayer *layer;
	int index;
	int old_length, new_length;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__GROWBUFFER_H_ */
