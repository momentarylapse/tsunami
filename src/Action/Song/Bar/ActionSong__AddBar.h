/*
 * ActionSong__AddBar.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_SONG_BAR_ACTIONSONG__ADDBAR_H_
#define SRC_ACTION_SONG_BAR_ACTIONSONG__ADDBAR_H_

#include "../../Action.h"
#include "../../../Data/Song.h"

class ActionSong__AddBar: public Action
{
public:
	ActionSong__AddBar(int index, BarPattern &Bar);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index;
	BarPattern bar;
};

#endif /* SRC_ACTION_SONG_BAR_ACTIONSONG__ADDBAR_H_ */
