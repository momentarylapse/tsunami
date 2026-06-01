/*
 * ActionManager.cpp
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#include "ActionManager.h"
#include "Action.h"
#include "ActionGroup.h"
#include "MergableAction.h"
#include "Data.h"
#include <lib/os/time.h>
#include <lib/os/msg.h>
#include <cassert>


namespace history {

class DummyActionGroup : public ActionGroup {
	string _name_;
public:
	DummyActionGroup(const string &name) {
		_name_ = name;
	}
	string name() const override { return _name_; }
};

ActionManager::ActionManager(Data *_data) {
	data = _data;
	cur_group = nullptr;
	_preview = nullptr;
	lock_level = 0;
	timer = new os::Timer;
	reset();
}

ActionManager::~ActionManager() {
	reset();
}

void ActionManager::reset() {
	history.clear();
	cur_pos = 0;
	save_pos = 0;
	cur_level = 0;
	enabled = true;
	cur_group_level = 0;
	cur_group = nullptr;
	_preview = nullptr;
	out_changed.notify();
}

void ActionManager::_truncate_future_history() {
	history.resize(cur_pos);
}

void ActionManager::_add_to_history(Action *a) {
	_truncate_future_history();

	if (timer->get() < 2.0f)
		if (_try_merge_into_head(a))
			return;

	history.add(a);
	cur_pos ++;
}

void ActionManager::enable(bool _enabled) {
	enabled = _enabled;
}

bool ActionManager::is_enabled() {
	return enabled;
}

bool ActionManager::_try_merge_into_head(Action *a) {
	if (history.num < 1)
		return false;

	auto *aa = dynamic_cast<MergableAction*>(a);
	auto *bb = dynamic_cast<MergableAction*>(history.back());
	if (!aa or !bb)
		return false;

	if (!bb->absorb(aa))
		return false;

	delete a;
	return true;
}

void ActionManager::_edit_start() {
	data->out_before_change.notify();
	_lock();
}

void ActionManager::_edit_end() {
	_unlock();
	data->out_after_change.notify();
	data->out_changed.notify();
	out_changed.notify();
}


void *ActionManager::execute(xfer<Action> a) {
	clear_preview();

	if (cur_group)
		return cur_group->add_sub_action(a, data);

	try {
		_edit_start();
		auto r = a->execute_logged(data);

		if (enabled and !a->is_trivial()) {
			_add_to_history(a);
			data->out_changed();
		}

		_edit_end();
		out_do_action.notify();
		return r;
	} catch(ActionException& e) {
		e.add_parent(a->name());
		msg_error(e.message);
		a->abort(data);
		out_failed.notify(e.message);
		return nullptr;
	}
}

bool ActionManager::undo() {
	if (!undoable())
		return false;

	clear_preview();
	_edit_start();
	history[-- cur_pos]->undo_logged(data);
	prev_action = history[cur_pos];
	_edit_end();
	out_undo_action.notify();
	return true;
}



bool ActionManager::redo() {
	if (!redoable())
		return false;

	clear_preview();
	_edit_start();
	prev_action = history[cur_pos];
	history[cur_pos ++]->redo_logged(data);
	_edit_end();
	out_redo_action.notify();
	return true;
}

bool ActionManager::undoable() {
	return enabled and (cur_pos > 0);
}

bool ActionManager::redoable() {
	return enabled and (cur_pos < history.num);
}



void ActionManager::begin_group(const string &name) {
	clear_preview();
	if (!cur_group) {
		cur_group = new DummyActionGroup(name);
		_edit_start();
	}
	cur_group_level ++;
}

void ActionManager::end_group() {
	cur_group_level --;
	assert(cur_group_level >= 0);

	if (cur_group_level == 0) {
		_add_to_history(cur_group.give());
		_edit_end();
	}
}

void ActionManager::mark_current_as_save() {
	save_pos = cur_pos;
	out_saved.notify();
	out_changed.notify();
}


bool ActionManager::is_save() {
	return (cur_pos == save_pos);
}


bool ActionManager::preview(Action *a) {
	clear_preview();
	try {
		a->execute_logged(data);
		_preview = a;
	} catch(ActionException &e) {
		e.add_parent(a->name());
		a->abort(data);
		delete a;
		out_failed.notify(e.message);
		return false;
	}
	return true;
}


void ActionManager::clear_preview() {
	if (_preview) {
		_preview->undo_logged(data);
		delete(_preview);
		_preview = nullptr;
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

string ActionManager::get_current_action() const {
	if (prev_action)
		return prev_action->name();
	return "<none>";
}

}

