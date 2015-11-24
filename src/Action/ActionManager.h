/*
 * ActionManager.h
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#ifndef ACTIONMANAGER_H_
#define ACTIONMANAGER_H_

#include "../lib/base/base.h"
#include "../lib/hui/HuiTimer.h"
#include "Action.h"
#include "../Data/Data.h"

class Data;
class Action;
class ActionGroup;

class ActionManager
{
public:
	ActionManager(Data *_data);
	virtual ~ActionManager();
	void reset();
	void enable(bool enabled);
	bool isEnabled();

	void *execute(Action *a);
	void undo();
	void redo();

	void beginActionGroup();
	void endActionGroup();

	bool undoable();
	bool redoable();
	bool isSave();
	void markCurrentAsSave();

private:
	void truncate();
	bool merge(Action *a);
	void add(Action *a);
	Data *data;
	Array<Action*> action;
	int cur_pos;
	int save_pos;

	int cur_level;
	bool enabled;

	// group
	int cur_group_level;
	ActionGroup *cur_group;

	HuiTimer timer;
};

#endif /* ACTIONMANAGER_H_ */
