/*
 * ActionSampleDelete.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionSampleDelete.h"

#include <assert.h>
#include "../../Data/Song.h"

ActionSampleDelete::ActionSampleDelete(Sample *s)
{
	sample = s;
	index = -1;
}

ActionSampleDelete::~ActionSampleDelete()
{
	if (!sample->owner)
		delete(sample);
}

void *ActionSampleDelete::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(sample->ref_count == 0);
	index = -1;
	for (int i=0; i<a->samples.num; i++)
		if (a->samples[i] == sample)
			index = i;
	assert(index >= 0);

	sample->notify(sample->MESSAGE_DELETE);
	a->samples.erase(index);
	sample->owner = NULL;

	a->notify(a->MESSAGE_DELETE_SAMPLE);
	return NULL;
}

void ActionSampleDelete::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	sample->owner = a;
	a->samples.insert(sample, index);

	a->notify(a->MESSAGE_ADD_SAMPLE);
}

