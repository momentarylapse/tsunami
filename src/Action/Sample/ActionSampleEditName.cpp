/*
 * ActionSampleEditName.cpp
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#include "ActionSampleEditName.h"

#include "../../Data/Sample.h"

ActionSampleEditName::ActionSampleEditName(shared<Sample> s, const string &name) {
	sample = s;
	new_value = name;
	old_value = s->name;
}

void *ActionSampleEditName::execute(Data *d) {
	sample->name = new_value;
	sample->notify(sample->MESSAGE_CHANGE_BY_ACTION);

	return nullptr;
}

void ActionSampleEditName::undo(Data *d) {
	sample->name = old_value;
	sample->notify(sample->MESSAGE_CHANGE_BY_ACTION);
}


bool ActionSampleEditName::mergable(Action *a) {
	auto *aa = dynamic_cast<ActionSampleEditName*>(a);
	if (!aa)
		return false;
	return (aa->sample == sample.get());
}


