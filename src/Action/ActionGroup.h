/*
 * ActionGroup.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONGROUP_H_
#define ACTIONGROUP_H_

#include "Action.h"
#include "../Data/Data.h"

class Data;
class ActionManager;

class ActionGroup: public Action {
	friend class ActionManager;
public:
	ActionGroup();

	//string name() override { return "-group-"; }

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

#endif /* ACTIONGROUP_H_ */
