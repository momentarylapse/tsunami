/*
 * ActionSong__DeleteBar.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_SONG_BAR_ACTIONSONG__DELETEBAR_H_
#define SRC_ACTION_SONG_BAR_ACTIONSONG__DELETEBAR_H_

#include "../../Action.h"
#include "../../../Data/Song.h"

class ActionSong__DeleteBar: public Action
{
public:
	ActionSong__DeleteBar(int index);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	BarPattern bar;
	int index;
};

#endif /* SRC_ACTION_SONG_BAR_ACTIONSONG__DELETEBAR_H_ */
