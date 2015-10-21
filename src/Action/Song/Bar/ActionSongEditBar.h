/*
 * ActionSongEditBar.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGEDITBAR_H_
#define ACTIONSONGEDITBAR_H_

#include "../../Action.h"
#include "../../../Data/Song.h"

class ActionSongEditBar: public Action
{
public:
	ActionSongEditBar(int index, BarPattern &bar, bool affect_midi);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	BarPattern bar;
	int index;
	bool affect_midi;
};

#endif /* ACTIONSONGEDITBAR_H_ */
