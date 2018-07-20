/*
 * Data.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef SRC_DATA_DATA_H_
#define SRC_DATA_DATA_H_

#include "../lib/base/base.h"
#include "../Stuff/Observable.h"

class ActionManager;
class Action;
class Session;

class Data : public Observable<VirtualBase>
{
public:
	Data(Session *session);
	virtual ~Data();

	static const string MESSAGE_FINISHED_LOADING;

	virtual void _cdecl reset() = 0;

	void resetHistory();
	void *execute(Action *a);
	void _cdecl undo();
	void _cdecl redo();
	void _cdecl beginActionGroup();
	void _cdecl endActionGroup();

	Session *session;
	string filename;
	int file_time;
	bool binary_file_format;

	ActionManager *action_manager;
	bool history_enabled();

	void lock();
	bool try_lock();
	void unlock();
};

#endif /* SRC_DATA_DATA_H_ */
