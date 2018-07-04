/*
 * ActionTrack__AbsorbBuffer.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ABSORBBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ABSORBBUFFER_H_

#include "../../Action.h"

class TrackLayer;

class ActionTrack__AbsorbBuffer : public Action
{
public:
	ActionTrack__AbsorbBuffer(TrackLayer *l, int _dest, int _src);
	virtual ~ActionTrack__AbsorbBuffer();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	TrackLayer *layer;
	int dest, src;
	int dest_old_length;
	int src_offset, src_length;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ABSORBBUFFERBOX_H_ */
