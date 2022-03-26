/*
 * Action.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#pragma once

#include "../data/Data.h"
#include "../lib/base/pointer.h"

class Data;

class Action {
public:
	Action() {}
	virtual ~Action() {}

	virtual string name() const { return "?"; };

	virtual void *execute(Data *d) = 0;
	virtual void undo(Data *d) = 0;
	virtual void redo(Data *d);

	virtual bool is_trivial() { return false; }
};

