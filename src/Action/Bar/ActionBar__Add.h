/*
 * ActionBar__Add.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_BAR_ACTIONBAR__ADD_H_
#define SRC_ACTION_BAR_ACTIONBAR__ADD_H_

#include "../Action.h"
#include "../../Rhythm/BarCollection.h"

class ActionBar__Add: public Action
{
public:
	ActionBar__Add(int index, BarPattern &Bar);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index;
	BarPattern bar;
};

#endif /* SRC_ACTION_BAR_ACTIONBAR__ADD_H_ */
