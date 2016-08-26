/*
 * ActionSong__EditBar.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_SONG_BAR_ACTIONSONG__EDITBAR_H_
#define SRC_ACTION_SONG_BAR_ACTIONSONG__EDITBAR_H_

#include "../../Action.h"
#include "../../../Data/Rhythm.h"

class ActionSong__EditBar : public Action
{
public:
	ActionSong__EditBar(int index, BarPattern &bar);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	BarPattern bar;
	int index;
};

#endif /* SRC_ACTION_SONG_BAR_ACTIONSONG__EDITBAR_H_ */
