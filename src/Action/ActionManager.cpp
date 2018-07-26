/*
 * ActionManager.cpp
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#include "ActionManager.h"
#include "Action.h"
#include "ActionMergable.h"
#include "ActionGroup.h"
#include "../Data/Data.h"
#include <assert.h>

ActionManager::ActionManager(Data *_data)
{
	data = _data;
	cur_group = nullptr;
	lock_level = 0;
	//mutex = new Mutex;
	timer = new hui::Timer;
	reset();
}

ActionManager::~ActionManager()
{
	reset();
	//delete(mutex);
	delete(timer);
}

void ActionManager::reset()
{
	for (Action *a: action)
		delete(a);
	action.clear();
	cur_pos = 0;
	save_pos = 0;
	cur_level = 0;
	enabled = true;
	cur_group_level = 0;
	if (cur_group)
		delete(cur_group);
	cur_group = nullptr;
}


void ActionManager::truncate()
{
	// truncate future history
	for (int i=cur_pos;i<action.num;i++)
		delete(action[i]);
	action.resize(cur_pos);
}

void ActionManager::add(Action *a)
{
	truncate();

	if (timer->get() < 2.0f)
		if (merge(a))
			return;

	action.add(a);
	cur_pos ++;
}

bool ActionManager::merge(Action *a)
{
	if (action.num < 1)
		return false;

	ActionMergableBase *aa = dynamic_cast<ActionMergableBase*>(a);
	ActionMergableBase *bb = dynamic_cast<ActionMergableBase*>(action.back());
	if (!aa or !bb)
		return false;

	if (!bb->absorb(aa))
		return false;

	delete(a);
	return true;
}


void *ActionManager::execute(Action *a)
{
	if (cur_group)
		return cur_group->addSubAction(a, data);

	lock();
	void *r = a->execute(data);
	unlock();

	if (enabled and !a->is_trivial())
		add(a);

	data->notify();
	return r;
}



void ActionManager::undo()
{
	if (!undoable())
		return;

	lock();
	action[-- cur_pos]->undo(data);
	unlock();

	data->notify();
}



void ActionManager::redo()
{
	if (!redoable())
		return;

	lock();
	action[cur_pos ++]->redo(data);
	unlock();

	data->notify();
}

bool ActionManager::undoable()
{
	return enabled and (cur_pos > 0);
}



bool ActionManager::redoable()
{
	return enabled and (cur_pos < action.num);
}



void ActionManager::markCurrentAsSave()
{
	save_pos = cur_pos;
}



bool ActionManager::isSave()
{
	return (cur_pos == save_pos);
}

void ActionManager::enable(bool _enabled)
{
	enabled = _enabled;
}

bool ActionManager::isEnabled()
{
	return enabled;
}

class DummyActionGroup : public ActionGroup
{
	virtual void build(Data *d){}
};

void ActionManager::beginActionGroup()
{
	if (!cur_group){
		cur_group = new DummyActionGroup;
		lock();
	}
	cur_group_level ++;
}

void ActionManager::endActionGroup()
{
	cur_group_level --;
	assert(cur_group_level >= 0);

	if (cur_group_level == 0){
		ActionGroup *g = cur_group;
		cur_group = nullptr;
		//execute(g);
		add(g);
		data->notify();
		unlock();
	}
}

void ActionManager::lock()
{
	if (lock_level == 0)
		mtx.lock();
	lock_level ++;
}

void ActionManager::unlock()
{
	lock_level --;
	if (lock_level == 0)
		mtx.unlock();
}

