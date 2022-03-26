/*
 * ActionBar__Add.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_BAR_ACTIONBAR__ADD_H_
#define SRC_ACTION_BAR_ACTIONBAR__ADD_H_

#include "../Action.h"

class Bar;

class ActionBar__Add: public Action {
public:
	ActionBar__Add(int index, Bar *bar);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	shared<Bar> bar;
};

#endif /* SRC_ACTION_BAR_ACTIONBAR__ADD_H_ */
