/*
 * ActionBar__Edit.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_BAR_ACTIONBAR__EDIT_H_
#define SRC_ACTION_BAR_ACTIONBAR__EDIT_H_

#include "../Action.h"

class ActionBar__Edit : public Action
{
public:
	ActionBar__Edit(int index, int length, int num_beats, int num_sub_beats, Array<int> &pattern);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int length, num_beats, num_sub_beats;
	Array<int> pattern;
	int index;
};

#endif /* SRC_ACTION_BAR_ACTIONBAR__EDIT_H_ */
