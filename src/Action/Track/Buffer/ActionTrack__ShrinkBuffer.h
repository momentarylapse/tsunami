/*
 * ActionTrack__ShrinkBuffer.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__SHRINKBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__SHRINKBUFFER_H_

#include "../../Action.h"
#include "../../../Data/Track.h"

class ActionTrack__ShrinkBuffer : public Action
{
public:
	ActionTrack__ShrinkBuffer(Track *t, int _level_no, int _index, int _length);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	int index;
	int level_no;
	int old_length, new_length;
	AudioBuffer buf;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__SHRINKBUFFER_H_ */
