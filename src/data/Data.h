/*
 * Data.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#pragma once

#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../lib/os/path.h"
#include "../lib/pattern/Observable.h"
#include <shared_mutex>

class ActionManager;
class Action;
class Session;

class Data : public Sharable<obs::Node<VirtualBase>> {
public:
	explicit Data(Session *session);
	virtual ~Data();

	static const string MESSAGE_START_LOADING;
	static const string MESSAGE_FINISHED_LOADING;
	static const string MESSAGE_BEFORE_CHANGE;
	static const string MESSAGE_AFTER_CHANGE;
	obs::Source out_start_loading{this, "start-loading"};
	obs::Source out_finished_loading{this, "finished-loading"};
	obs::Source out_before_change{this, "before-change"};
	obs::Source out_after_change{this, "after-change"};

	virtual void _cdecl reset() = 0;

	void reset_history();
	void *execute(Action *a);
	bool _cdecl undo();
	bool _cdecl redo();
	void _cdecl begin_action_group(const string &name);
	void _cdecl end_action_group();

	Session *session;
	Path filename;
	int file_time;
	bool binary_file_format;

	ActionManager *action_manager;
	bool history_enabled();


	std::shared_timed_mutex mtx;
	void lock();
	bool try_lock();
	void unlock();


	struct XMessageData {
		int i[8];
	} x_message_data;
};

