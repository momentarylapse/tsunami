/*
 * ActionSongDeleteBar.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGDELETEBAR_H_
#define ACTIONSONGDELETEBAR_H_

#include "../../Action.h"
#include "../../../Data/Song.h"

class ActionSongDeleteBar: public Action
{
public:
	ActionSongDeleteBar(int index, bool affect_midi);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	BarPattern bar;
	int index;
	bool affect_midi;
};

#endif /* ACTIONSONGDELETEBAR_H_ */
