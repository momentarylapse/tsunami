/*
 * ActionManager.h
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#pragma once

#include "Action.h"
#include "Data.h"
#include <lib/pattern/Observable.h>
#include <lib/base/pointer.h>

namespace os {
	class Timer;
}

namespace history {

class Data;
class Action;
class ActionGroup;

class ActionManager : public obs::Node<VirtualBase> {
	friend class Action;
public:
	explicit ActionManager(Data *_data);
	~ActionManager() override;

	obs::source out_do_action{this, "do-action"};
	obs::source out_undo_action{this, "undo-action"};
	obs::source out_redo_action{this, "redo-action"};
	obs::xsource<string> out_failed{this, "failed"};
	obs::source out_saved{this, "saved"};

	void reset();
	void enable(bool enabled);
	bool is_enabled();

	void* execute(xfer<Action> a);
	bool undo();
	bool redo();

	void begin_group(const string& name);
	void end_group();

	bool preview(Action* a);
	void clear_preview();

	bool undoable();
	bool redoable();
	bool is_save();
	void mark_current_as_save();

	string get_current_action() const;

private:
	void _truncate_future_history();
	bool _try_merge_into_head(Action *a);
	void _add_to_history(Action *a);
	Data* data;
	owned_array<Action> history;
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

	// preview
	Action* _preview;

	// for merging
	owned<os::Timer> timer;
};

}

