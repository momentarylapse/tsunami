/*
 * ActionGroup.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#pragma once

#include "Action.h"

namespace history {

class Data;
class ActionManager;

class ActionGroup: public Action {
	friend class ActionManager;
public:
	ActionGroup();
	~ActionGroup() override;

	virtual void* compose(Data* d) {
		return nullptr;
	}

	void* execute(Data* d) override;
	void undo(Data* d) override;
	void redo(Data* d) override;

	void abort(Data* d) override;
	bool is_trivial() const override;

protected:
	void* add_sub_action(xfer<Action> a, Data* d);

private:
	owned_array<Action> actions;
};

}
