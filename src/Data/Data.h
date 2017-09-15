/*
 * Data.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef DATA_H_
#define DATA_H_

#include "../lib/base/base.h"
#include "../Action/Action.h"
#include "../Action/ActionManager.h"
#include "../Stuff/Observable.h"

class ActionManager;
class Action;
class Observable;

class Data : public Observable
{
public:
	Data();
	virtual ~Data();

	static const string MESSAGE_FINISHED_LOADING;

	virtual void _cdecl reset() = 0;
	virtual bool _cdecl load(const string &_filename, bool deep = true) = 0;
	virtual bool _cdecl save(const string &_filename) = 0;

	void resetHistory();
	void *execute(Action *a);
	void _cdecl undo();
	void _cdecl redo();

	string filename;
	int file_time;
	bool binary_file_format;

	ActionManager *action_manager;

	void lock();
	bool try_lock();
	void unlock();
};

#endif /* DATA_H_ */
