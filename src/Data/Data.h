/*
 * Data.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef DATA_H_
#define DATA_H_

#include "../lib/file/file.h"
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

	virtual void Reset() = 0;
	virtual bool Load(const string &_filename, bool deep = true) = 0;
	virtual bool Save(const string &_filename) = 0;

	virtual void PostActionUpdate(){}

	void ResetHistory();
	void *Execute(Action *a);
	void Undo();
	void Redo();

	string filename;
	int file_time;
	bool binary_file_format;

	ActionManager *action_manager;
};

#endif /* DATA_H_ */
