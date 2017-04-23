/*
 * ActionBar__Edit.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_BAR_ACTIONBAR__EDIT_H_
#define SRC_ACTION_BAR_ACTIONBAR__EDIT_H_

#include "../Action.h"
#include "../../Data/Rhythm.h"

class ActionBar__Edit : public Action
{
public:
	ActionBar__Edit(int index, BarPattern &bar);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	BarPattern bar;
	int index;
};

#endif /* SRC_ACTION_BAR_ACTIONBAR__EDIT_H_ */
