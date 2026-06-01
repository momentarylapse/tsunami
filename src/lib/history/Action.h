/*
 * Action.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#pragma once

#include "Data.h"

namespace history {

class Data;
class ActionManager;

class ActionException {
public:
	explicit ActionException(const string& _message) {
		message = _message;
	}
	string message;
	Array<string> location;
	void add_parent(const string& loc);
	string where() const;
};

class Action {
public:
	Action();
	virtual ~Action();

	virtual string name() const { return "???"; }

	virtual bool allowed(Data* d) {
		return true;
	}
	virtual void abort(Data* d) {
		undo_logged(d);
	}
	virtual bool is_trivial() const {
		return false;
	}

	virtual void* execute(Data* d) = 0;
	virtual void undo(Data* d) = 0;
	virtual void redo(Data* d) {
		execute(d);
	}

	void* execute_logged(Data* d);
	void undo_logged(Data* d);
	void redo_logged(Data* d);
};

}
