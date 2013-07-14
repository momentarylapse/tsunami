/*
 * ActionTrack__CutBufferBox.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef ACTIONTRACK__CUTBUFFERBOX_H_
#define ACTIONTRACK__CUTBUFFERBOX_H_

#include "../../Action.h"
#include "../../../Data/Track.h"

class ActionTrack__CutBufferBox : public Action
{
public:
	ActionTrack__CutBufferBox(Track *t, int _level_no, int _index, int _offset);
	virtual ~ActionTrack__CutBufferBox();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	int index;
	int level_no;
	int offset;
};

#endif /* ACTIONTRACK__CUTBUFFERBOX_H_ */
