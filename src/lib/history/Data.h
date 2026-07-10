#pragma once


#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/os/path.h>
#include <lib/obs/Observable.h>
#include "Action.h"
#include "ActionManager.h"
#include <shared_mutex>

namespace history {

class ActionManager;
class Action;

class Data : public Sharable<obs::Node<VirtualBase>> {
public:
	explicit Data(int type);
	~Data() override;

	obs::source out_start_loading{this, "start-loading"};
	obs::source out_finished_loading{this, "finished-loading"};
	obs::source out_before_change{this, "before-change"};
	obs::source out_after_change{this, "after-change"};

	virtual void reset() = 0;

	void reset_history();
	void* execute(Action* a);
	bool undo();
	bool redo();
	void begin_action_group(const string& name);
	void end_action_group();
	bool undoable() const;
	bool redoable() const;
	bool history_enabled();


	std::shared_timed_mutex mtx;
	void lock();
	bool try_lock();
	void unlock();
	void lock_shared();
	bool try_lock_shared();
	void unlock_shared();

	Path filename;
	int file_time;
	int type;

	ActionManager* action_manager;
};

}
