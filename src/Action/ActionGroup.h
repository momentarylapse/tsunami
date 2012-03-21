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

class ActionGroup: public Action
{
public:
	ActionGroup();
	virtual ~ActionGroup();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

protected:
	void AddSubAction(Action *a, Data *d);
	virtual void *execute_return(Data *d);

private:
	Array<Action*> action;
};

#endif /* ACTIONGROUP_H_ */
