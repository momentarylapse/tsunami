/*
 * ActionManager.cpp
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#include "ActionManager.h"
#include "Action.h"
#include "../Data/Data.h"

ActionManager::ActionManager(Data *_data)
{
	data = _data;
	cur_pos = 0;
	save_pos = 0;
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
	add(a);
	return a->execute_and_notify(data);
}



void ActionManager::Undo()
{
	if (Undoable())
		action[-- cur_pos]->undo_and_notify(data);
}



void ActionManager::Redo()
{
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




