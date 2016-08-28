/*
 * ActionSong__DeleteLevel.h
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_SONG_LEVEL_ACTIONSONG__DELETELEVEL_H_
#define SRC_ACTION_SONG_LEVEL_ACTIONSONG__DELETELEVEL_H_

#include "../../Action.h"

class ActionSong__DeleteLevel : public Action
{
public:
	ActionSong__DeleteLevel(int index);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
private:
	int index;
	string name;
};

#endif /* SRC_ACTION_SONG_LEVEL_ACTIONSONG__DELETELEVEL_H_ */
