/*
 * ActionTrackEditBar.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITBAR_H_
#define ACTIONTRACKEDITBAR_H_

#include "../../Action.h"
#include "../../../Data/Track.h"

class ActionTrackEditBar: public Action
{
public:
	ActionTrackEditBar(Track *t, int index, BarPattern &bar);
	virtual ~ActionTrackEditBar();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	BarPattern bar;
	int track_no;
	int index;
};

#endif /* ACTIONTRACKEDITBAR_H_ */
