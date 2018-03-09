/*
 * Data.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef SRC_DATA_DATA_H_
#define SRC_DATA_DATA_H_

#include "../lib/base/base.h"
#include "../Action/Action.h"
#include "../Action/ActionManager.h"
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

	Session *session;
	string filename;
	int file_time;
	bool binary_file_format;

	ActionManager *action_manager;

	void lock();
	bool try_lock();
	void unlock();
};

#endif /* SRC_DATA_DATA_H_ */
