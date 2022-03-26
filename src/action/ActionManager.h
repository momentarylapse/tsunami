/*
 * ActionManager.h
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#pragma once

#include "../lib/base/base.h"
#include "Action.h"
#include "../data/Data.h"
#include "../stuff/Observable.h"

class Data;
class Action;
class ActionGroup;
//class Mutex;
namespace hui {
	class Timer;
}

class ActionManager : public Observable<VirtualBase> {
	//friend class Data;
public:
	ActionManager(Data *_data);
	virtual ~ActionManager();


	static const string MESSAGE_DO_ACTION;
	static const string MESSAGE_UNDO_ACTION;
	static const string MESSAGE_REDO_ACTION;

	void reset();
	void enable(bool enabled);
	bool is_enabled();

	void *execute(Action *a);
	bool undo();
	bool redo();

	void group_begin(const string &name);
	void group_end();

	bool undoable();
	bool redoable();
	bool is_save();
	void mark_current_as_save();

	string get_current_action() const;

private:
	void _truncate_future_history();
	bool _try_merge_into_head(Action *a);
	void _add_to_history(Action *a);
	Data *data;
	owned_array<Action> action;
	int cur_pos;
	int save_pos;

	int cur_level;
	bool enabled;

	void _edit_start();
	void _edit_end();

	// mutex
	void _lock();
	void _unlock();
	int lock_level;

	// group
	int cur_group_level;
	owned<ActionGroup> cur_group;
	Action *prev_action = nullptr;

	// for merging
	owned<hui::Timer> timer;
};

