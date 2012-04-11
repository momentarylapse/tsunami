/*
 * ActionTrackEditBuffer.h
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITBUFFER_H_
#define ACTIONTRACKEDITBUFFER_H_

#include "Action.h"
#include "../Data/Track.h"

class ActionTrackEditBuffer : public Action
{
public:
	ActionTrackEditBuffer(Track *t, Range _range);
	virtual ~ActionTrackEditBuffer();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

private:
	int track_no, sub_no;
	Range range;
	BufferBox box;
};

#endif /* ACTIONTRACKEDITBUFFER_H_ */
