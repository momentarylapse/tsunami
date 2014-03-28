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

class ActionGroup: public Action
{
	friend class ActionManager;
public:
	ActionGroup();
	virtual ~ActionGroup();

	virtual string name(){	return "-group-";	}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
	virtual void redo(Data *d);

	virtual bool is_trivial();

protected:
	void *AddSubAction(Action *a, Data *d);
	virtual void *execute_return(Data *d);

private:
	Array<Action*> action;
};

#endif /* ACTIONGROUP_H_ */
