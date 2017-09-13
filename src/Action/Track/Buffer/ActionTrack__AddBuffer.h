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

class ActionTrack__AddBuffer : public Action
{
public:
	ActionTrack__AddBuffer(Track *t, int _level_no, int index, Range r);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	int index;
	int level_no;
	Range range;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ADDBUFFER_H_ */
