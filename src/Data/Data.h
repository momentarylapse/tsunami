/*
 * Data.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef SRC_DATA_DATA_H_
#define SRC_DATA_DATA_H_

#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../lib/file/path.h"
#include "../Stuff/Observable.h"
#include <shared_mutex>

class ActionManager;
class Action;
class Session;

class Data : public Sharable<Observable<VirtualBase>> {
public:
	explicit Data(Session *session);
	virtual ~Data();

	static const string MESSAGE_START_LOADING;
	static const string MESSAGE_FINISHED_LOADING;
	static const string MESSAGE_BEFORE_CHANGE;
	static const string MESSAGE_AFTER_CHANGE;

	virtual void _cdecl reset() = 0;

	void reset_history();
	void *execute(Action *a);
	bool _cdecl undo();
	bool _cdecl redo();
	void _cdecl begin_action_group();
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
};

#endif /* SRC_DATA_DATA_H_ */
