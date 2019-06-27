/*
 * ActionSampleDelete.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionSampleDelete.h"

#include <assert.h>
#include "../../Data/Song.h"
#include "../../Data/Sample.h"

ActionSampleDelete::ActionSampleDelete(Sample *s) {
	sample = s->_pointer_ref();
	index = -1;
}

ActionSampleDelete::~ActionSampleDelete() {
	sample->_pointer_unref();
}

void *ActionSampleDelete::execute(Data *d) {
	Song *a = dynamic_cast<Song*>(d);
	assert(sample->ref_count == 0);
	index = -1;
	for (int i=0; i<a->samples.num; i++)
		if (a->samples[i] == sample)
			index = i;
	assert(index >= 0);

	sample->fake_death();
	a->samples.erase(index);
	sample->unset_owner();

	a->notify(a->MESSAGE_DELETE_SAMPLE);
	return nullptr;
}

void ActionSampleDelete::undo(Data *d) {
	Song *a = dynamic_cast<Song*>(d);
	sample->set_owner(a);
	a->samples.insert(sample, index);

	a->notify(a->MESSAGE_ADD_SAMPLE);
}

