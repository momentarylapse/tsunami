/*
 * ActionTrack__DeleteBuffer.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__DELETEBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__DELETEBUFFER_H_

#include "../../Action.h"
#include "../../../Data/Audio/AudioBuffer.h"

class TrackLayer;

class ActionTrack__DeleteBuffer : public Action
{
public:
	ActionTrack__DeleteBuffer(TrackLayer *l, int _index);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	TrackLayer *layer;
	int index;
	AudioBuffer buf;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__DELETEBUFFER_H_ */
