/*
 * ActionSampleDelete.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionSampleDelete.h"

#include <assert.h>
#include "../../data/Song.h"
#include "../../data/Sample.h"

namespace tsunami {

ActionSampleDelete::ActionSampleDelete(shared<Sample> s) {
	sample = s;
	index = -1;
}

void *ActionSampleDelete::execute(Data *d) {
	Song *a = dynamic_cast<Song*>(d);
	assert(sample->ref_count == 0);
	index = weak(a->samples).find(sample.get());
	assert(index >= 0);

	sample->fake_death();
	a->samples.erase(index);
	sample->unset_owner();

	a->out_sample_list_changed.notify();
	return nullptr;
}

void ActionSampleDelete::undo(Data *d) {
	Song *a = dynamic_cast<Song*>(d);
	sample->set_owner(a);
	a->samples.insert(sample, index);

	a->out_sample_list_changed.notify();
}

}

