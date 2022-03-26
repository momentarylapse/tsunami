/*
 * ActionSampleAdd.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionSampleAdd.h"

#include <assert.h>
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Sample.h"

ActionSampleAdd::ActionSampleAdd(shared<Sample> s) {
	sample = s;
}

void *ActionSampleAdd::execute(Data *d) {
	Song *a = dynamic_cast<Song*>(d);
	sample->set_owner(a);
	a->samples.add(sample);
	a->notify(a->MESSAGE_ADD_SAMPLE);
	return sample.get();
}

void ActionSampleAdd::undo(Data *d) {
	Song *a = dynamic_cast<Song*>(d);
	assert(sample->ref_count == 0);
	sample->fake_death();
	a->samples.pop();
	sample->unset_owner();
	a->notify(a->MESSAGE_DELETE_SAMPLE);
}

