/*
 * Action__ShiftData.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_BAR_ACTION__SHIFTDATA_H_
#define SRC_ACTION_BAR_ACTION__SHIFTDATA_H_

#include "../Action.h"
#include "../../Data/Song.h"

class Action__ShiftData: public Action
{
public:
	Action__ShiftData(int offset, int shift, int mode);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	void do_shift(Song *s, int delta);

private:
	int offset, shift;
	int mode;
};

#endif /* SRC_ACTION_BAR_ACTION__SHIFTDATA_H_ */
