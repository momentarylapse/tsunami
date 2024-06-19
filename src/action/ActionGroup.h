/*
 * ActionGroup.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#pragma once

#include "Action.h"
#include "../data/Data.h"


namespace tsunami {

class Data;
class ActionManager;

class ActionGroup: public Action {
	friend class ActionManager;
public:
	ActionGroup();

	virtual void build(Data *d) = 0;

	void *execute(Data *d) override;
	void undo(Data *d) override;
	void redo(Data *d) override;

	bool is_trivial() override;

protected:
	void *add_sub_action(Action *a, Data *d);
	virtual void *execute_return(Data *d);

private:
	owned_array<Action> action;
};

}

