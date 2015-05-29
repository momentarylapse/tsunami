/*
 * ActionAudioDeleteSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionAudioDeleteSample.h"
#include "../../../Data/AudioFile.h"
#include <assert.h>

ActionAudioDeleteSample::ActionAudioDeleteSample(int _index)
{
	index = _index;
	sample = NULL;
}

ActionAudioDeleteSample::~ActionAudioDeleteSample()
{
	if (sample)
		if (!sample->owner)
			delete(sample);
}

void *ActionAudioDeleteSample::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0);
	assert(index < a->samples.num);
	sample = a->samples[index];
	assert(sample->ref_count == 0);

	sample->notify(sample->MESSAGE_DELETE);
	a->samples.erase(index);
	sample->owner = NULL;

	a->notify(a->MESSAGE_DELETE_SAMPLE);
	return NULL;
}

void ActionAudioDeleteSample::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	sample->owner = a;
	a->samples.insert(sample, index);

	a->notify(a->MESSAGE_ADD_SAMPLE);
}

