/*
 * ActionTrack__SplitBufferBox.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef ACTIONTRACK__SPLITBUFFERBOX_H_
#define ACTIONTRACK__SPLITBUFFERBOX_H_

#include "../../Action.h"
#include "../../../Data/Track.h"

class ActionTrack__SplitBufferBox : public Action
{
public:
	ActionTrack__SplitBufferBox(Track *t, int _level_no, int _index, int _offset);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	int index;
	int level_no;
	int offset;
};

#endif /* ACTIONTRACK__SPLITBUFFERBOX_H_ */
