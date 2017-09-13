/*
 * ActionTrack__DeleteBuffer.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__DELETEBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__DELETEBUFFER_H_

#include "../../Action.h"
#include "../../../Data/Track.h"

class ActionTrack__DeleteBuffer : public Action
{
public:
	ActionTrack__DeleteBuffer(Track *t, int level_no, int _index);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	int level_no;
	int index;
	AudioBuffer buf;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__DELETEBUFFER_H_ */
