/*
 * ActionSong__ShiftData.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_SONG_BAR_ACTIONSONG__SHIFTDATA_H_
#define SRC_ACTION_SONG_BAR_ACTIONSONG__SHIFTDATA_H_

#include "../../Action.h"
#include "../../../Data/Song.h"

class ActionSong__ShiftData: public Action
{
public:
	ActionSong__ShiftData(int offset, int shift);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	void do_shift(Song *s, int delta);

private:
	int offset, shift;
};

#endif /* SRC_ACTION_SONG_BAR_ACTIONSONG__SHIFTDATA_H_ */
