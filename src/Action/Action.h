/*
 * Action.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef ACTION_H_
#define ACTION_H_

#include "../Data/Data.h"

class Data;

class Action
{
public:
	Action();
	virtual ~Action();

	virtual void *execute(Data *d) = 0;
	virtual void undo(Data *d) = 0;
	virtual void redo(Data *d);

	void *execute_and_notify(Data *d);
	void undo_and_notify(Data *d);
	void redo_and_notify(Data *d);
};

#endif /* ACTION_H_ */
