/*
 * ActionSampleEditName.cpp
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#include "ActionSampleEditName.h"

#include "../../data/Sample.h"

ActionSampleEditName::ActionSampleEditName(shared<Sample> s, const string &name) {
	sample = s;
	new_value = name;
	old_value = s->name;
}

void *ActionSampleEditName::execute(Data *d) {
	sample->name = new_value;
	sample->out_changed_by_action.notify();

	return nullptr;
}

void ActionSampleEditName::undo(Data *d) {
	sample->name = old_value;
	sample->out_changed_by_action.notify();
}


bool ActionSampleEditName::mergable(Action *a) {
	auto *aa = dynamic_cast<ActionSampleEditName*>(a);
	if (!aa)
		return false;
	return (aa->sample == sample.get());
}


