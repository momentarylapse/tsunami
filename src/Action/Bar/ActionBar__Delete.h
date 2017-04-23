/*
 * ActionBar__Delete.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_BAR_ACTIONBAR__DELETE_H_
#define SRC_ACTION_BAR_ACTIONBAR__DELETE_H_

#include "../Action.h"
#include "../../Data/Song.h"

class ActionBar__Delete: public Action
{
public:
	ActionBar__Delete(int index);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	BarPattern bar;
	int index;
};

#endif /* SRC_ACTION_BAR_ACTIONBAR__DELETE_H_ */
