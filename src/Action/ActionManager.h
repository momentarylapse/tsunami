/*
 * ActionManager.h
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#ifndef ACTIONMANAGER_H_
#define ACTIONMANAGER_H_

#include "../lib/file/file.h"
#include "Action.h"
#include "../Data/Data.h"

class Data;
class Action;

class ActionManager
{
public:
	ActionManager(Data *_data);
	virtual ~ActionManager();
	void Reset();

	void *Execute(Action *a);
	void add(Action *a);
	void Undo();
	void Redo();

	bool Undoable();
	bool Redoable();
	bool IsSave();
	void MarkCurrentAsSave();

	Data *data;

	Array<Action*> action;
	int cur_pos;
	int save_pos;
};

#endif /* ACTIONMANAGER_H_ */
