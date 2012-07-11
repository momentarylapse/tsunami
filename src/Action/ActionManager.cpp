/*
 * ActionManager.cpp
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#include "ActionManager.h"
#include "Action.h"
#include "ActionGroup.h"
#include "../Data/Data.h"
#include <assert.h>

ActionManager::ActionManager(Data *_data)
{
	data = _data;
	cur_group = NULL;
	Reset();
}

ActionManager::~ActionManager()
{
	Reset();
}

void ActionManager::Reset()
{
	foreach(action, a)
		delete(a);
	action.clear();
	cur_pos = 0;
	save_pos = 0;
	cur_level = 0;
	enabled = true;
	cur_group_level = 0;
	if (cur_group)
		delete(cur_group);
	cur_group = NULL;
}



void ActionManager::add(Action *a)
{
	// truncate history
	for (int i=cur_pos;i<action.num;i++)
		delete(action[i]);
	action.resize(cur_pos);

	action.add(a);
	cur_pos ++;
}



void *ActionManager::Execute(Action *a)
{
	if (enabled){
		if (cur_group)
			return cur_group->AddSubAction(a, data);
		add(a);
		return a->execute_and_notify(data);
	}else
		return a->execute(data);
}



void ActionManager::Undo()
{
	if (!enabled)
		return;
	if (Undoable())
		action[-- cur_pos]->undo_and_notify(data);
}



void ActionManager::Redo()
{
	if (!enabled)
		return;
	if (Redoable())
		action[cur_pos ++]->redo_and_notify(data);
}

bool ActionManager::Undoable()
{
	return (cur_pos > 0);
}



bool ActionManager::Redoable()
{
	return (cur_pos < action.num);
}



void ActionManager::MarkCurrentAsSave()
{
	save_pos = cur_pos;
}



bool ActionManager::IsSave()
{
	return (cur_pos == save_pos);
}

void ActionManager::Enable(bool _enabled)
{
	enabled = _enabled;
}


void ActionManager::BeginActionGroup()
{
	if (!cur_group){
		cur_group = new ActionGroup;
	}
	cur_group_level ++;
}

void ActionManager::EndActionGroup()
{
	cur_group_level --;
	assert(cur_group_level >= 0);

	if (cur_group_level == 0){
		ActionGroup *g = cur_group;
		cur_group = NULL;
		Execute(g);
	}
}

