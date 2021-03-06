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
#include "../lib/file/msg.h"
#include "../lib/hui/Timer.h"
#include "../Data/Data.h"
#include <assert.h>

ActionManager::ActionManager(Data *_data) {
	data = _data;
	cur_group = nullptr;
	lock_level = 0;
	timer = new hui::Timer;
	reset();
}

ActionManager::~ActionManager() {
	reset();
}

void ActionManager::reset() {
	action.clear();
	cur_pos = 0;
	save_pos = 0;
	cur_level = 0;
	enabled = true;
	cur_group_level = 0;
	cur_group = nullptr;
	notify();
}


void ActionManager::_truncate_future_history() {
	action.resize(cur_pos);
}

void ActionManager::_add_to_history(Action *a) {
	_truncate_future_history();

	if (timer->get() < 2.0f)
		if (_try_merge_into_head(a))
			return;

	action.add(a);
	cur_pos ++;
}

bool ActionManager::_try_merge_into_head(Action *a) {
	if (action.num < 1)
		return false;

	auto *aa = dynamic_cast<ActionMergableBase*>(a);
	auto *bb = dynamic_cast<ActionMergableBase*>(action.back());
	if (!aa or !bb)
		return false;

	if (!bb->absorb(aa))
		return false;

	delete a;
	return true;
}

void ActionManager::_edit_start() {
	data->notify(Data::MESSAGE_BEFORE_CHANGE);
	_lock();
}

void ActionManager::_edit_end() {
	_unlock();
	data->notify(Data::MESSAGE_AFTER_CHANGE);
	data->notify();
	notify();
}


void *ActionManager::execute(Action *a) {
	if (cur_group)
		return cur_group->add_sub_action(a, data);

	_edit_start();
	auto r = a->execute(data);

	if (enabled and !a->is_trivial())
		_add_to_history(a);

	_edit_end();
	return r;
}



void ActionManager::undo() {
	if (!undoable())
		return;

	_edit_start();
	action[-- cur_pos]->undo(data);
	_edit_end();
}



void ActionManager::redo() {
	if (!redoable())
		return;

	_edit_start();
	action[cur_pos ++]->redo(data);
	_edit_end();
}

bool ActionManager::undoable() {
	return enabled and (cur_pos > 0);
}



bool ActionManager::redoable() {
	return enabled and (cur_pos < action.num);
}



void ActionManager::mark_current_as_save() {
	save_pos = cur_pos;
	notify();
}



bool ActionManager::is_save() {
	return (cur_pos == save_pos);
}

void ActionManager::enable(bool _enabled) {
	enabled = _enabled;
}

bool ActionManager::is_enabled() {
	return enabled;
}

class DummyActionGroup : public ActionGroup {
	virtual void build(Data *d) {}
};

void ActionManager::group_begin() {
	if (!cur_group){
		cur_group = new DummyActionGroup;
		_edit_start();
	}
	cur_group_level ++;
}

void ActionManager::group_end() {
	cur_group_level --;
	assert(cur_group_level >= 0);

	if (cur_group_level == 0) {
		_add_to_history(cur_group.check_out());
		_edit_end();
	}
}

void ActionManager::_lock() {
	if (lock_level == 0)
		data->lock();
	else
		msg_error("LOCK LEVEL > 1");
	lock_level ++;
}

void ActionManager::_unlock() {
	lock_level --;
	if (lock_level == 0)
		data->unlock();
}

