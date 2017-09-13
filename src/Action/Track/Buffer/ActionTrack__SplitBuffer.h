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
	ActionTrack__SplitBuffer(Track *t, int _level_no, int _index, int _offset);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	int index;
	int level_no;
	int offset;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__SPLITBUFFER_H_ */
