/*
 * ActionBar__Delete.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_BAR_ACTIONBAR__DELETE_H_
#define SRC_ACTION_BAR_ACTIONBAR__DELETE_H_

#include "../Action.h"

class Bar;

class ActionBar__Delete: public Action
{
public:
	ActionBar__Delete(int index);
	~ActionBar__Delete();

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Bar *bar;
	int index;
};

#endif /* SRC_ACTION_BAR_ACTIONBAR__DELETE_H_ */
