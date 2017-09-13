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
	ActionTrackEditBuffer(Track *t, int _level_no, Range _range);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
	virtual void redo(Data *d);

private:
	int track_no;
	Range range;
	int level_no;
	AudioBuffer box;
	int index;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACKEDITBUFFER_H_ */
