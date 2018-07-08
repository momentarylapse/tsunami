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
	ActionTrack__AbsorbBuffer(TrackLayer *l, int dest, int src);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	TrackLayer *layer;
	int dest, src;
	int dest_old_length;
	int src_offset, src_length;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ABSORBBUFFERBOX_H_ */
