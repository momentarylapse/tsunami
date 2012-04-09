/*
 * ActionTrack__CutBufferBox.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef ACTIONTRACK__CUTBUFFERBOX_H_
#define ACTIONTRACK__CUTBUFFERBOX_H_

#include "Action.h"
#include "../Data/Track.h"

class ActionTrack__CutBufferBox : public Action
{
public:
	ActionTrack__CutBufferBox(Track *t, int _index, int _offset);
	virtual ~ActionTrack__CutBufferBox();

	void *execute(Data *d);
	void undo(Data *d);

private:
	int track_no, sub_no;
	int index;
	int offset;
};

#endif /* ACTIONTRACK__CUTBUFFERBOX_H_ */
