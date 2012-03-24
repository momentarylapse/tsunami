/*
 * ActionTrack__AbsorbBufferBox.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef ACTIONTRACK__ABSORBBUFFERBOX_H_
#define ACTIONTRACK__ABSORBBUFFERBOX_H_

#include "Action.h"

class ActionTrack__AbsorbBufferBox : public Action
{
public:
	ActionTrack__AbsorbBufferBox(int _track_no, int _dest, int _src);
	virtual ~ActionTrack__AbsorbBufferBox();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

private:
	int track_no;
	int dest, src;
};

#endif /* ACTIONTRACK__ABSORBBUFFERBOX_H_ */
