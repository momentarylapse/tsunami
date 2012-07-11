/*
 * ActionTrack__AbsorbBufferBox.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef ACTIONTRACK__ABSORBBUFFERBOX_H_
#define ACTIONTRACK__ABSORBBUFFERBOX_H_

#include "../Action.h"
#include "../../Data/Track.h"

class ActionTrack__AbsorbBufferBox : public Action
{
public:
	ActionTrack__AbsorbBufferBox(Track *t, int _level_no, int _dest, int _src);
	virtual ~ActionTrack__AbsorbBufferBox();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no, sub_no;
	int level_no;
	int dest, src;
	int dest_old_length;
	int src_offset, src_length;
};

#endif /* ACTIONTRACK__ABSORBBUFFERBOX_H_ */
