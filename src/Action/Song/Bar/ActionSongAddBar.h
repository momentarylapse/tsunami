/*
 * ActionSongAddBar.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGADDBAR_H_
#define ACTIONSONGADDBAR_H_

#include "../../Action.h"
#include "../../../Data/Song.h"

class ActionSongAddBar: public Action
{
public:
	ActionSongAddBar(int index, BarPattern &Bar, bool affect_midi);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index;
	BarPattern bar;
	bool affect_midi;
};

#endif /* ACTIONSONGADDBAR_H_ */
