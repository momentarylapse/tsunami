/*
 * ActionTrackGrowBufferBox.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKGROWBUFFERBOX_H_
#define ACTIONTRACKGROWBUFFERBOX_H_

#include "../Action.h"
#include "../../Data/Track.h"

class ActionTrack__GrowBufferBox : public Action
{
public:
	ActionTrack__GrowBufferBox(Track *t, int _level_no, int _index, int _new_length);
	virtual ~ActionTrack__GrowBufferBox();

	void *execute(Data *d);
	void undo(Data *d);

private:
	int track_no, sub_no, index;
	int level_no;
	int old_length, new_length;
};

#endif /* ACTIONTRACKGROWBUFFERBOX_H_ */
