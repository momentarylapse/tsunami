/*
 * ActionTrack__AddBufferBox.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef ACTIONTRACK__ADDBUFFERBOX_H_
#define ACTIONTRACK__ADDBUFFERBOX_H_

#include "../Action.h"
#include "../../Data/Track.h"

class ActionTrack__AddBufferBox : public Action
{
public:
	ActionTrack__AddBufferBox(Track *t, int index, Range r);
	virtual ~ActionTrack__AddBufferBox();

	void *execute(Data *d);
	void undo(Data *d);

private:
	int track_no, sub_no;
	int index;
	Range range;
};

#endif /* ACTIONTRACK__ADDBUFFERBOX_H_ */
