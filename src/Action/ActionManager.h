/*
 * ActionManager.h
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#ifndef ACTIONMANAGER_H_
#define ACTIONMANAGER_H_

#include "../lib/base/base.h"
#include "Action.h"
#include "../Data/Data.h"
#include "../Stuff/Observable.h"
#include <shared_mutex>

class Data;
class Action;
class ActionGroup;
//class Mutex;
namespace hui {
	class Timer;
}

class ActionManager : public Observable<VirtualBase> {
	friend class Data;
public:
	ActionManager(Data *_data);
	virtual ~ActionManager();
	void reset();
	void enable(bool enabled);
	bool is_enabled();

	void *execute(Action *a);
	void undo();
	void redo();

	void group_begin();
	void group_end();

	bool undoable();
	bool redoable();
	bool is_save();
	void mark_current_as_save();

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

	// mutex
	void lock();
	void unlock();
	bool try_lock();
	int lock_level;

	// group
	int cur_group_level;
	ActionGroup *cur_group;

	// for merging
	hui::Timer *timer;

	//Mutex *mutex;
	std::shared_timed_mutex mtx;
};

#endif /* ACTIONMANAGER_H_ */
