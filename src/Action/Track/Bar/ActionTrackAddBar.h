/*
 * ActionTrackAddBar.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKADDBAR_H_
#define ACTIONTRACKADDBAR_H_

#include "../../Action.h"
#include "../../../Data/Track.h"

class ActionTrackAddBar: public Action
{
public:
	ActionTrackAddBar(Track *t, int index, Bar &Bar);
	virtual ~ActionTrackAddBar();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index;
	Bar bar;
	int track_no;
};

#endif /* ACTIONTRACKADDBAR_H_ */
