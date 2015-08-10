/*
 * ActionTrackDeleteBar.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKDELETEBAR_H_
#define ACTIONTRACKDELETEBAR_H_

#include "../../Action.h"
#include "../../../Data/Track.h"

class ActionTrackDeleteBar: public Action
{
public:
	ActionTrackDeleteBar(Track *t, int index, bool affect_midi);
	virtual ~ActionTrackDeleteBar();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	BarPattern bar;
	int track_no;
	int index;
	bool affect_midi;
};

#endif /* ACTIONTRACKDELETEBAR_H_ */
