/*
 * ActionGroup.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionGroup.h"

namespace history {

ActionGroup::ActionGroup() = default;
ActionGroup::~ActionGroup() = default;

void *ActionGroup::add_sub_action(xfer<Action> a, Data* d) {
	void* r = nullptr;
	try {
		r = a->execute_logged(d);
		if (!a->is_trivial())
			actions.add(a);
	} catch(ActionException& e) {
		e.add_parent(a->name());
		a->abort(d);
		throw;
	}
	return r;
}


void *ActionGroup::execute(Data* d) {
	void* r = compose(d);

	// no need to execute sub actions ... done during compose()
	/*for (action, a)
		a->execute_logged(d);*/
	return r;
}



void ActionGroup::undo(Data* d) {
	foreachb(Action* a, weak(actions))
		a->undo_logged(d);
}



void ActionGroup::redo(Data* d) {
	for (Action* a: weak(actions))
		a->redo_logged(d);
}

void ActionGroup::abort(Data* d) {
	foreachb(Action* a, weak(actions))
		a->undo_logged(d);
}

bool ActionGroup::is_trivial() const {
	return actions.num == 0;
}

}


