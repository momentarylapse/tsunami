/*
 * ActionManager.h
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#pragma once

#include "Action.h"
#include "../lib/base/base.h"
#include "../lib/pattern/Observable.h"
#include "../data/Data.h"

//class Mutex;
namespace os {
	class Timer;
}

namespace tsunami {

class Data;
class Action;
class ActionGroup;

class ActionManager : public obs::Node<VirtualBase> {
	//friend class Data;
public:
	ActionManager(Data *_data);
	virtual ~ActionManager();

	obs::source out_do_action{this, "do-action"};
	obs::source out_undo_action{this, "undo-action"};
	obs::source out_redo_action{this, "redo-action"};

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
	owned<os::Timer> timer;
};

}

